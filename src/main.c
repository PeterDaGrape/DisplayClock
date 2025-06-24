#include <stdio.h>
#include <stdlib.h>     //exit()
#include <signal.h>     //signal()
#include <pthread.h>	//pthread_create()


#include "EPD_2in13_V3.h"

#include "DisplayHandler.h"
#include "GUI_Paint.h"
#include "GUI_BMPfile.h"
#include "GT1151.h"
#include "fonts.h"
#include <stdbool.h>

#include "MainMenuView.h"
#include "ClockDisplay.h"
#include "MandelBrotView.h"

#define MAX_PARTIAL_REFRESHES 7
#define DISPLAY_TIMEOUT 120
#define NUM_VIEWS 1


static pthread_t touchThread;
UBYTE touchThreadContinueFlag = 1;	
extern GT1151_Dev Dev_Now, Dev_Old;




void *touchThreadHandler(void *arg) {

	while(touchThreadContinueFlag) {
		if(DEV_Digital_Read(INT) == 0) {
			Dev_Now.Touch = 1;
		}
		else {
			Dev_Now.Touch = 0;
		}
		DEV_Delay_ms(1);
	}
	printf("thread:exit\r\n");
	pthread_exit(NULL);
}

//Affects the epd driver
void stopHandler(int signo)
{
    //System Exit
    printf("\r\nHandler:exit\r\n");
	EPD_2in13_V3_Sleep();
	DEV_Delay_ms(2000);
	touchThreadContinueFlag = 0;
	pthread_join(touchThread, NULL);
    DEV_ModuleExit();
    exit(0);
}

extern int IIC_Address;

int main(void)
{

    //setup view here before touch controller begins
    MainMenuView_Init();
    ClockView_Init();
    MandelBrotView_Init();

    viewManager.switchView(&viewManager, &mainMenuView);

    //Setup hardware
    IIC_Address = 0x14;
    signal(SIGINT, stopHandler);
    DEV_ModuleInit();

	pthread_create(&touchThread, NULL, touchThreadHandler, NULL);

    EPD_2in13_V3_Init(EPD_2IN13_V3_FULL);
 
    EPD_2in13_V3_Clear();
    GT_Init();
	DEV_Delay_ms(100);


    //Create a new image cache
    UBYTE *image;
    UWORD Imagesize = ((EPD_2in13_V3_WIDTH % 8 == 0)? (EPD_2in13_V3_WIDTH / 8 ): (EPD_2in13_V3_WIDTH / 8 + 1)) * EPD_2in13_V3_HEIGHT;

    //image = malloc(Imagesize + 256); 

    if((image = (UBYTE *)malloc(Imagesize)) == NULL) {
        printf("Failed to apply for black memory...\r\n");
        return -1;
    }
    

    Paint_NewImage(image, EPD_2in13_V3_WIDTH, EPD_2in13_V3_HEIGHT, ROTATE_270, WHITE);
    Paint_SelectImage(image);
    Paint_SetMirroring(MIRROR_ORIGIN);
    Paint_Clear(WHITE);

    //GUI_ReadBmp("./rsc/pics/photo.bmp", 0, 0);
    
    viewManager.currentView -> update(viewManager.currentView);
    viewManager.currentView -> draw(viewManager.currentView);
	EPD_2in13_V3_Display(image);
	EPD_2in13_V3_Init(EPD_2IN13_V3_PART);
	EPD_2in13_V3_Display_Partial_Wait(image);
    

    int refreshTimer = 0;
    bool sleep = false;
    time_t sleepTimer = time(NULL) + DISPLAY_TIMEOUT;



    printf("Init succeeded \n");
    while (1) {
        //printf("Current time: %d, sleep timeout: %d\n", time(NULL), sleepTimer);

        // time stuff
        time_t now = time(NULL);

		if(GT_Scan()==0 && (Dev_Now.X[0] != Dev_Old.X[0] && Dev_Now.Y[0] != Dev_Old.Y[0])) { // No new touch


            Debug("Touch occurred \n");
            if (Dev_Now.TouchpointFlag) {
                Dev_Now.TouchpointFlag = 0;
                viewManager.currentView -> touch(viewManager.currentView, EPD_2in13_V3_HEIGHT - Dev_Now.Y[0], Dev_Now.X[0]);
            }   
        }

        viewManager.currentView -> update(viewManager.currentView);
        if (viewManager.drawRequired) {
            Paint_Clear(WHITE);
            viewManager.currentView -> draw(viewManager.currentView);
            Debug("Drawing %s \n", viewManager.currentView->appName);

        }
        
        if (now > sleepTimer && !sleep) {
            printf("Entering sleep\n");
            sleep = true;
            EPD_2in13_V3_Sleep();
        }

        if (viewManager.drawRequired) {
            printf("Refreshing... \n");
            viewManager.drawRequired = false;
            sleepTimer = now + DISPLAY_TIMEOUT;
            if (refreshTimer++ > MAX_PARTIAL_REFRESHES || viewManager.forceFullRefresh || sleep) {
                refreshTimer = 0;
                printf("I'm fully refreshing \n");
                EPD_2in13_V3_Init(EPD_2IN13_V3_FULL);
                EPD_2in13_V3_Display_Base(image);
                EPD_2in13_V3_Init(EPD_2IN13_V3_PART);
                viewManager.forceFullRefresh = false;
                
            } else {
                printf("I'm partially refreshing \n");
                EPD_2in13_V3_Display_Partial_Wait(image);
            }
            
            sleep = false;
        } else {
            DEV_Delay_ms(1);
        }

    }

    return 0;
}



