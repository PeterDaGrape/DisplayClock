
#include "DisplayHandler.h"
#include <stdio.h>
#include <stdlib.h>     //exit()
#include <stdbool.h>
#include "GUI_Paint.h"
#include "GUI_BMPfile.h"
#include "fonts.h"
#include "Debug.h"
#include "GT1151.h"
#include "EPD_2in13_V3.h"

#ifndef RSC_PATH
#define RSC_PATH "./rsc"
#endif

#define ICON_PATH(file) RSC_PATH "/pic/" file

#include "MainMenuView.h"

#define HOME_BUTTON_SIZE 32

#define NUM_REGIONS 1

extern ViewManager viewManager;
extern View mainMenuView;

typedef struct ClockData {
    char formattedTime[100];
    char formattedDate[32];
    int min;
} ClockData;

ClockData* clockData = NULL;

static struct TouchRegion touchRegions[NUM_REGIONS];

void ClockDraw(struct View* self) {
 
    ClockData* data = (ClockData*)self->data;
    if (!self || !self->data) {
        Debug("Data is invalid! \n");
        return;
    }
    //Paint_DrawString_EN(0, 0, "CLOCK", &Font16, WHITE, BLACK);
    Paint_DrawString_EN(0, 0, data -> formattedTime, &Font20, WHITE, BLACK);
    Paint_DrawString_EN(0, 20, data -> formattedDate, &Font16, WHITE, BLACK);


    // draw touch points
    for (int i = 0; i < NUM_REGIONS; i++) {

        Paint_DrawRectangle(touchRegions[i].xStart, touchRegions[i].yStart, touchRegions[i].xStop, touchRegions[i].yStop, BLACK, 1, 1);
        printf("%i %i %i %i \n", touchRegions[i].xStart, touchRegions[i].yStart, touchRegions[i].xStop, touchRegions[i].yStop);

    }
    
    GUI_ReadBmp(ICON_PATH("home.bmp"), 0, EPD_2in13_V3_WIDTH - HOME_BUTTON_SIZE, HOME_BUTTON_SIZE, HOME_BUTTON_SIZE);

}

void ClockUpdate(struct View* self) {


    ClockData* data = (ClockData*)self->data;
    if (!self || !self->data) {
        Debug("Data is invalid! \n");
        return;
    }
    time_t now = time(NULL);
    struct tm *local_time = localtime(&now);

    if (local_time->tm_min != data->min) {

        strftime(data->formattedTime, sizeof(data->formattedTime), "%H:%M", local_time);
        strftime(data->formattedDate, sizeof(data->formattedDate), "%a %d/%m/%y", local_time);
        data->min = local_time->tm_min;
        viewManager.drawRequired = true;
    }    
}

void ClockTouch(struct View* self, int x, int y) {

    ClockData* data = (ClockData*)self->data;
    if (!self || !self->data) {
        Debug("Data is invalid! \n");
        return;
    }
    for (int i = 0; i < NUM_REGIONS; i++) {
        if (touchRegions[i].testRegion(&touchRegions[i], x, y)) {
            printf("Region %i was touched \n", i);

            switch (i) {
                case 0:
                // home button
                Debug("Going home \n");
                viewManager.switchView(&viewManager, &mainMenuView);
                break;
            }

        }
    }

    printf("I was touched \n");
    printf("Location x:%d y:%d \n", x, y);

}

struct View clockView = {
    .touchRegions = touchRegions,
    .numTouchRegions = NUM_REGIONS,
    .touch = ClockTouch,
    .data = NULL,
    .appName = "Clock",
    .iconPath = ICON_PATH("clock.bmp"),
    .draw = ClockDraw,
    .update = ClockUpdate 
};

void ClockView_Init(void) {
    clockData = malloc(sizeof(ClockData));

    for (int i = 0; i < NUM_REGIONS; i++) {
        touchRegions[i].setRegion = TouchRegion_setRegion;
        touchRegions[i].testRegion = TouchRegion_testRegion;
    }

    //This is how the touch regions are set! 
    touchRegions[0].setRegion(&touchRegions[0], 0, EPD_2in13_V3_WIDTH - HOME_BUTTON_SIZE, HOME_BUTTON_SIZE + 1, HOME_BUTTON_SIZE);

    clockData -> min = -1;

    clockView.data = clockData;
    
}