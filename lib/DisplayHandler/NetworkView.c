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

// Usage:
#include "MainMenuView.h"

#define HOME_BUTTON_SIZE 32
#define NUM_REGIONS 1

#define PING_HOST "google.com"

extern ViewManager viewManager;
extern View mainMenuView;

typedef struct NetworkData {
    char pingStr[128];
    bool isUpdated;
    bool threadActive;
    pthread_mutex_t mutex;
} NetworkData;

NetworkData* networkData = NULL;

static struct TouchRegion touchRegions[NUM_REGIONS];

pthread_t thread;



void* checkPing(void* arg) {
    NetworkData *data = (NetworkData*)arg;
    if (!data) {
        Debug("Data is invalid! \n");
        return 0;
    }

    while (data -> threadActive) {

        char buffer[128];
        FILE *fp;

        // Create the command to ping the host
        char command[256];
        snprintf(command, sizeof(command), "ping -c 1 %s", PING_HOST);

        // Run the command and open a pipe to read the output
        fp = popen(command, "r");
        if (fp == NULL) {
            printf("Failed to run ping command\n");
            snprintf(data->pingStr, sizeof(data->pingStr), "Ping: FAILED");
            data->isUpdated = true;

            continue;
        }

        pthread_mutex_lock(&data->mutex);
        // Read and process the output
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            // Filter for the line containing 'time=' (this indicates a reply)
            if (strstr(buffer, "time=")) {
                char *time_ptr = strstr(buffer, "time=");
                if (time_ptr) {
                    // Move pointer past "time="
                    time_ptr += 5;
                    // Copy up to " ms" or end of string
                    char time_value[32] = {0};
                    int i = 0;
                    while (time_ptr[i] && time_ptr[i] != ' ' && i < (int)sizeof(time_value) - 1) {
                        time_value[i] = time_ptr[i];
                        i++;
                    }
                    time_value[i] = '\0';
                    snprintf(data->pingStr, sizeof(data->pingStr), "Ping: %s ms", time_value);
                    data->isUpdated = true;
                }
            }

        }
        /*
        if (!data -> isUpdated) {
            Debug("Something wrong with isUpdated \n");
            continue;
        }
        */
        pthread_mutex_unlock(&data->mutex);

        pclose(fp);

        DEV_Delay_ms(4000);

    }
}


static void draw(struct View* self) {
    NetworkData* data = (NetworkData*)self->data;
    if (!self || !self->data) {
        Debug("Data is invalid! \n");
        return;
    }

    //printf("Drawing the network view \n ");

    Paint_DrawString_EN(0, 0, "Network Manager", &Font16, WHITE, BLACK);

    pthread_mutex_lock(&data->mutex);
    if (data -> pingStr) {
        Paint_DrawString_EN(0, 16, data -> pingStr, &Font12, WHITE, BLACK);
    }
    pthread_mutex_unlock(&data->mutex);
    
    // draw touch points
    for (int i = 0; i < NUM_REGIONS; i++) {

        Paint_DrawRectangle(touchRegions[i].xStart, touchRegions[i].yStart, touchRegions[i].xStop, touchRegions[i].yStop, BLACK, 1, 1);
        printf("%i %i %i %i \n", touchRegions[i].xStart, touchRegions[i].yStart, touchRegions[i].xStop, touchRegions[i].yStop);

    }
    
    GUI_ReadBmp(ICON_PATH("home.bmp"), 0, EPD_2in13_V3_WIDTH - HOME_BUTTON_SIZE, HOME_BUTTON_SIZE, HOME_BUTTON_SIZE);
    printf("Drawn network\n ");
}


static void update(struct View* self) {
    NetworkData* data = (NetworkData*)self->data;
    if (!self || !self->data) {
        Debug("Data is invalid! \n");
        return;
    }

    pthread_mutex_lock(&data->mutex);
    if (data -> isUpdated && data -> pingStr) {
        data -> isUpdated = false;
        viewManager.drawRequired = true;
    }
    pthread_mutex_unlock(&data->mutex);
}

static void touch(struct View* self, int x, int y) {

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
}

static void openView(struct View* self) {
    NetworkData* data = (NetworkData*)self->data;
    if (!self || !self->data) {
        Debug("Data is invalid! \n");
        return;
    }

    data -> threadActive = true;

    pthread_create(&thread, NULL, checkPing, self -> data);
    printf("Started ping thread \n");
}

static void closeView(struct View* self) {
    NetworkData* data = (NetworkData*)self->data;
    if (!self || !self->data) {
        Debug("Data is invalid! \n");
        return;
    }
    data -> threadActive = false;
 
    pthread_join(thread, NULL);
    printf("Closed ping thread \n");
}

struct View networkView = {
    .touchRegions = touchRegions,
    .numTouchRegions = NUM_REGIONS,
    .touch = touch,
    .data = NULL,
    .appName = "Network",
    .iconPath = "",
    .openView = openView,
    .closeView = closeView,
    .draw = draw,
    .update = update
};

void NetworkView_Init(void) {
    networkData = malloc(sizeof(NetworkData));




    for (int i = 0; i < NUM_REGIONS; i++) {
        touchRegions[i].setRegion = TouchRegion_setRegion;
        touchRegions[i].testRegion = TouchRegion_testRegion;
    }
    
    pthread_mutex_init(&networkData->mutex, NULL);

    //This is how the touch regions are set! 
    touchRegions[0].setRegion(&touchRegions[0], 0, EPD_2in13_V3_WIDTH - HOME_BUTTON_SIZE, HOME_BUTTON_SIZE + 1, HOME_BUTTON_SIZE);
    networkData -> threadActive = false;
    networkData -> isUpdated = false;

    strcpy(networkData->pingStr, "Waiting for ping...");

    networkView.data = networkData;
 
}