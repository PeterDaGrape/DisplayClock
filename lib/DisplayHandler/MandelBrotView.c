#include "DisplayHandler.h"
#include <stdio.h>
#include <stdlib.h>     //exit()
#include <stdbool.h>
#include "GUI_Paint.h"
#include "GUI_BMPfile.h"
#include "Debug.h"
#include "GT1151.h"
#include "EPD_2in13_V3.h"
#include <math.h>
#include <pthread.h>

#define HOME_BUTTON_SIZE 32
#define NUM_REGIONS 2

#define NUM_THREADS 4

#define MAX_ITERATIONS 100

pthread_t threads[NUM_THREADS];


typedef struct MandelBrotData {
    long double xStart;
    long double yStart;
    long double yStop;

} MandelBrotData;

extern ViewManager viewManager;
extern View mainMenuView;

MandelBrotData* mandelBrotData = NULL;

static struct TouchRegion touchRegions[NUM_REGIONS];

typedef struct {

    MandelBrotData* data;
    int threadNumber;

} ThreadStruct;

void* renderBand(void* arg) {

    ThreadStruct *threadStruct = arg;
    MandelBrotData *data = threadStruct -> data;
    int threadNumber = threadStruct -> threadNumber;

    
    
    int xStart = EPD_2in13_V3_HEIGHT / NUM_THREADS * threadNumber;
    int xStop = EPD_2in13_V3_HEIGHT / NUM_THREADS * (threadNumber + 1);

    printf("Thread %i startX: %i stopX: %i \n ", threadNumber, xStart, xStop);

    long double stepSize = (data -> yStop - data -> yStart) / EPD_2in13_V3_WIDTH;
    for (int x = xStart; x < xStop; x++) {

        long double xComplex = data -> xStart + x * stepSize;

        for (int y = 0; y < EPD_2in13_V3_WIDTH; y++) {
            long double yComplex = data -> yStart + y * stepSize;

            long double xN = 0;
            long double yN = 0;
            int i;
            for (i = 0; i < MAX_ITERATIONS; i++) {
                long double xNNext = xN * xN - yN * yN + xComplex;
                long double yNNext = 2 * xN * yN + yComplex;

                xN = xNNext;
                yN = yNNext;
                if (xN * xN + yN * yN > 4) {
                    break;
                }
            }  

            if (i == MAX_ITERATIONS) {

                Paint_DrawPoint(x, y, BLACK,1 , DOT_STYLE_DFT);
            }
        }
    }
}


void draw(struct View* self) {
    MandelBrotData* data = (MandelBrotData*)self->data;
    if (!self || !self->data) {
        Debug("Data is invalid! \n");
        return;
    }


    printf("Generating mandelbrot... \n");

    ThreadStruct threadStructs[NUM_THREADS];


    for (int i = 0; i < NUM_THREADS; i++) {
        threadStructs[i].data = data;
        threadStructs[i].threadNumber = i;
        pthread_create(&threads[i], NULL, renderBand, &threadStructs[i]);
    }


    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }




    // draw touch points
    for (int i = 0; i < NUM_REGIONS; i++) {

        Paint_DrawRectangle(touchRegions[i].xStart, touchRegions[i].yStart, touchRegions[i].xStop, touchRegions[i].yStop, BLACK, 1, 1);
        printf("%i %i %i %i \n", touchRegions[i].xStart, touchRegions[i].yStart, touchRegions[i].xStop, touchRegions[i].yStop);

    }
    GUI_ReadBmp("./rsc/pic/reset.bmp", EPD_2in13_V3_HEIGHT - HOME_BUTTON_SIZE, EPD_2in13_V3_WIDTH - HOME_BUTTON_SIZE, HOME_BUTTON_SIZE, HOME_BUTTON_SIZE);
 
    GUI_ReadBmp("./rsc/pic/home.bmp", 0, EPD_2in13_V3_WIDTH - HOME_BUTTON_SIZE, HOME_BUTTON_SIZE, HOME_BUTTON_SIZE);

}

void update(struct View* self) {

    MandelBrotData* data = (MandelBrotData*)self->data;
    if (!self || !self->data) {
        Debug("Data is invalid! \n");
        return;
    }

}

void touch(struct View* self, int x, int y) {

    MandelBrotData* data = (MandelBrotData*)self->data;


    for (int i = 0; i < NUM_REGIONS; i++) {
        if (touchRegions[i].testRegion(&touchRegions[i], x, y)) {
            printf("Region %i was touched \n", i);
            switch (i) {
                case 0:
                // home button
                Debug("Going home \n");
                viewManager.forceFullRefresh = true;
                viewManager.switchView(&viewManager, &mainMenuView);
                return;

                case 1:
                // refresh button
                Debug("Resetting set \n");
                viewManager.forceFullRefresh = true;
                viewManager.drawRequired = true;

                data -> xStart = -2;
                data -> yStart = -1.2;
                data -> yStop = 1.2; 
                return;
 
            }

        }
    }

    //Mandelbrot update stuff!

    long double stepSize = (data -> yStop - data -> yStart) / EPD_2in13_V3_WIDTH;

    long double xCmplxTap = data -> xStart + x * stepSize;
    long double yCmplxTap = data -> yStart + y * stepSize;

    stepSize /= 2;

    data -> xStart = (data -> xStart + xCmplxTap) / 2;
    data -> yStart = (data -> yStart + yCmplxTap) / 2;
    data -> yStop = data -> yStart + EPD_2in13_V3_WIDTH * stepSize;
    

    printf("I was touched \n");
    printf("Location x:%d y:%d \n", x, y);
    viewManager.drawRequired = true;
}

struct View mandelBrotView = {
    .touchRegions = touchRegions,
    .numTouchRegions = NUM_REGIONS,
    .touch = touch,
    .data = NULL,
    .appName = "MandelBrot",
    .iconPath = "./rsc/pic/mandelbrot.bmp",
    .draw = draw,
    .update = update 
};

void MandelBrotView_Init(void) {
    mandelBrotData = malloc(sizeof(MandelBrotData));

    for (int i = 0; i < NUM_REGIONS; i++) {
        touchRegions[i].setRegion = TouchRegion_setRegion;
        touchRegions[i].testRegion = TouchRegion_testRegion;
    }

    //This is how the touch regions are set! 
    touchRegions[0].setRegion(&touchRegions[0], 0, EPD_2in13_V3_WIDTH - HOME_BUTTON_SIZE, HOME_BUTTON_SIZE + 1, HOME_BUTTON_SIZE);
    touchRegions[1].setRegion(&touchRegions[1], EPD_2in13_V3_HEIGHT - HOME_BUTTON_SIZE, EPD_2in13_V3_WIDTH - HOME_BUTTON_SIZE, HOME_BUTTON_SIZE, HOME_BUTTON_SIZE);

    mandelBrotData -> xStart = -2;
    mandelBrotData -> yStart = -1.2;
    mandelBrotData -> yStop = 1.2;



    mandelBrotView.data = mandelBrotData;
    
}