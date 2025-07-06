#include "DisplayHandler.h"
#include <stdio.h>
#include <stdlib.h>     //exit()
#include <stdbool.h>
#include "GUI_Paint.h"
#include "GUI_BMPfile.h"
#include "fonts.h"
#include "MainMenuView.h"
#include "Debug.h"
#include "GT1151.h"
#include "EPD_2in13_V3.h"

#include "ClockDisplay.h"
#include "MandelBrotView.h"

#ifndef RSC_PATH
#define RSC_PATH "./rsc"
#endif

#define ICON_PATH(file) RSC_PATH "/pic/" file


#define DBG_REGIONS 0
#define NUM_REGIONS 2

#define NUM_VIEWS 3

#define NUM_PAGES 2


#define min(a,b) \
({ __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a > _b ? _b : _a; })

#define PANELS_PER_PAGE 2

#define HEADER_SIZE 16
#define FOOTER_SIZE 16

extern ViewManager viewManager;

extern View clockView;
extern View mandelBrotView;
extern View networkView;

extern GT1151_Dev Dev_Now;

typedef struct MainMenuData {
    int page;
    char formattedTime[100];
    int min;
} MainMenuData;

MainMenuData* mainMenuData = NULL;

static struct TouchRegion touchRegions[NUM_REGIONS];

View *views[NUM_VIEWS] = {&clockView, &mandelBrotView, &networkView};


void MainMenuDraw(struct View* self) {
 

	//GUI_ReadBmp("./rsc/pics/photo.bmp", 0, 0);

    Debug("Got to draw \n");

    MainMenuData* data = (MainMenuData*)self->data;
    if (!self || !self->data) {
        Debug("Data is invalid! \n");
        return;
    }
    // draw time and date...
    Paint_DrawString_EN(0, 0, data -> formattedTime, &Font16, WHITE, BLACK);

    // draw UI
    // draw panels

    for (int i = 0; i < PANELS_PER_PAGE; i++) {
        int panelWidth = EPD_2in13_V3_HEIGHT / PANELS_PER_PAGE;
        Paint_DrawRectangle(i * panelWidth, HEADER_SIZE, (i+1) * panelWidth, EPD_2in13_V3_WIDTH - FOOTER_SIZE, BLACK, 2, 0);

        int index = i + data->page * PANELS_PER_PAGE;

        if (index < NUM_VIEWS && views[index] != NULL && views[index]->appName != NULL) {
            Paint_DrawString_EN(i * panelWidth, EPD_2in13_V3_WIDTH - FOOTER_SIZE - 16, views[index]->appName, &Font16, WHITE, BLACK);

            int bodyHeight = EPD_2in13_V3_WIDTH - HEADER_SIZE - FOOTER_SIZE;

            int iconSize = min(panelWidth, bodyHeight);
            iconSize = 64;


            int panelStartX = EPD_2in13_V3_HEIGHT / (PANELS_PER_PAGE * 2) - iconSize / 2;

            if (fopen(views[index]-> iconPath, "rb") != NULL) {
                GUI_ReadBmp(views[index]-> iconPath, i * panelWidth + panelStartX, EPD_2in13_V3_WIDTH - FOOTER_SIZE - 16 - 64, iconSize, iconSize);
            } else {
                Debug("File %s does not exist \n", views[index]-> iconPath);
            }
            

        } else {
            Debug("Invalid view or appName at index %d\n", index);
        }

    }

    // draw arrows
    if (data -> page > 0) {
        printf("Drawing back\n");
        Paint_DrawString_EN(0, EPD_2in13_V3_WIDTH - FOOTER_SIZE, "<---", &Font16, WHITE, BLACK);
        Paint_DrawRectangle(0, EPD_2in13_V3_WIDTH, EPD_2in13_V3_HEIGHT / 3, EPD_2in13_V3_WIDTH - FOOTER_SIZE, BLACK, 2, 0);
    }    
    if (data -> page < NUM_PAGES - 1) {
        printf("Drawing forwards\n");
        Paint_DrawString_EN(2*EPD_2in13_V3_HEIGHT / 3, EPD_2in13_V3_WIDTH - FOOTER_SIZE, "--->", &Font16, WHITE, BLACK);
        Paint_DrawRectangle(2*EPD_2in13_V3_HEIGHT / 3, EPD_2in13_V3_WIDTH, EPD_2in13_V3_HEIGHT, EPD_2in13_V3_WIDTH - FOOTER_SIZE, BLACK, 2, 0);
    }
    if (DBG_REGIONS) {
        for (int i = 0; i < NUM_REGIONS; i++) {
            Paint_DrawRectangle(touchRegions[i].xStart, touchRegions[i].yStart, touchRegions[i].xStop, touchRegions[i].yStop, BLACK, 1, 0);
        }
    }
    Debug("Finished draw \n");
}

void MainMenuUpdate(struct View* self) {
    MainMenuData* data = (MainMenuData*)self->data;
    if (!self || !self->data) {
        Debug("Data is invalid! \n");
        return;
    }
    time_t now = time(NULL);
    struct tm *local_time = localtime(&now);

    if (local_time->tm_min != data->min) {

        strftime(data->formattedTime, sizeof(data->formattedTime), "%H:%M %a %d/%m/%y", local_time);
        data->min = local_time->tm_min;
        viewManager.drawRequired = true;
    }
}

void MainMenuTouch(struct View* self, int x, int y) {
    MainMenuData* data = (MainMenuData*)self->data;
    if (!self || !self->data) {
        Debug("Data is invalid! \n");
        return;
    }    
    printf("I was touched \n");
    printf("Location x:%d y:%d \n", x, y);

    for (int i = 0; i < PANELS_PER_PAGE; i++) {
        int panelWidth = EPD_2in13_V3_HEIGHT / PANELS_PER_PAGE;
        if (x >= i*panelWidth && x < (i+1) * panelWidth && y > HEADER_SIZE && y < EPD_2in13_V3_WIDTH - FOOTER_SIZE) {
            printf("Panel: %d was touched \n", i);

            int viewIndex = i + data -> page * PANELS_PER_PAGE;
            if (viewIndex < NUM_VIEWS && views[viewIndex] != NULL) {
                viewManager.switchView(&viewManager, views[viewIndex]);
                return;
            }
        }
    }


    if (touchRegions[0].testRegion(&touchRegions[0], x, y) && data -> page + 1 < NUM_PAGES) {
        printf("Forwards button pressed \n");
        data -> page += 1;
        viewManager.drawRequired = true;
    }

    if (touchRegions[1].testRegion(&touchRegions[1], x, y) && data -> page > 0) {
        printf("Back button pressed \n");
        data -> page -= 1;
        viewManager.drawRequired = true;
    }

}

struct View mainMenuView = {
    .touchRegions = touchRegions,
    .numTouchRegions = NUM_REGIONS,
    .touch = MainMenuTouch,
    .data = NULL,
    .appName = "Main Menu",
    .iconPath = "",
    .draw = MainMenuDraw,
    .update = MainMenuUpdate
};

void MainMenuView_Init(void) {
    mainMenuData = malloc(sizeof(MainMenuData));

    for (int i = 0; i < NUM_REGIONS; i++) {
        touchRegions[i].setRegion = TouchRegion_setRegion;
        touchRegions[i].testRegion = TouchRegion_testRegion;
    }

    //This is how the touch regions are set! 
    // Forwards page button
    touchRegions[0].setRegion(&mainMenuView.touchRegions[0], 2 * EPD_2in13_V3_HEIGHT / 3, EPD_2in13_V3_WIDTH - FOOTER_SIZE, 84, FOOTER_SIZE);
    touchRegions[1].setRegion(&mainMenuView.touchRegions[1], 0, EPD_2in13_V3_WIDTH - FOOTER_SIZE, 84, FOOTER_SIZE);


    mainMenuData -> min = -1;
    mainMenuData -> page = 0;
    mainMenuView.data = mainMenuData;
    
}