
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

<<<<<<< Updated upstream
=======


pthread_t weatherThread;


// Struct to hold response data
struct Memory {
    char *response;
    size_t size;
};

// Callback function to handle the data received from the API
static size_t ResponseCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t totalSize = size * nmemb;
    struct Memory *mem = (struct Memory *)userp;

    printf(". %zu %zu\n", size, nmemb);
    char *ptr = realloc(mem->response, mem->size + totalSize + 1);
    if (ptr == NULL) {
        printf("Not enough memory to allocate buffer.\n");
        return 0;
    }

    mem->response = ptr;
    memcpy(&(mem->response[mem->size]), contents, totalSize);
    mem->size += totalSize;
    mem->response[mem->size] = '\0';

    return totalSize;
}

char* get_weekday_abbrev(const char *date_str) {
    struct tm tm = {0};
    sscanf(date_str, "%d-%d-%d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday);
    tm.tm_year -= 1900; // Adjust year
    tm.tm_mon -= 1;     // Adjust month

    // Normalize time
    mktime(&tm);

    char *buffer = malloc(4);
    strftime(buffer, 4, "%a", &tm);
    return buffer;
}



void* weatherCheck(void* arg) {
    ClockData *data = (ClockData*)arg;    
    if (!data) {
        Debug("Data is invalid! \n");
        return 0;
    }

    WeatherData *weatherData = data -> weatherData;
    if (!weatherData) {
        Debug("weatherData is invalid! \n");
        return 0;
    }

    printf("Weather thread running \n ");

    CURL *curl;
    CURLcode res;

    struct Memory chunk;

    chunk.response = malloc(1);  // Initialize memory

    curl_global_init(CURL_GLOBAL_DEFAULT);

    
    char apiURL[256];
    
    sprintf(apiURL, "https://api.weatherapi.com/v1/forecast.json?key=%s&q=auto:ip&days=%i&aqi=no&alerts=no", API_KEY, FORECAST_DAYS);

    
    // Initialize libcurl
    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, apiURL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ResponseCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);         // 10 seconds max for the whole request
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);   // 5 seconds max to connect

    time_t lastTime = -1;
    while (weatherData -> threadActive) {

        time_t now = time(NULL);

        if (curl && now - lastTime > 60 * 60 * 3) {
            lastTime = now;

            chunk.size = 0;             // No data yet

            // Perform the request
            res = curl_easy_perform(curl);
            curl_easy_cleanup(curl);
            

            // Check for errors
            if (res != CURLE_OK) {
                fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            } else {

                struct json_object *forecast_obj, *forecastday_arr;
                struct json_object *parsed_json = json_tokener_parse(chunk.response);
                if (!parsed_json) {
                    snprintf(weatherData->weatherString, sizeof(weatherData->weatherString), "Weather parse error");
                    weatherData->weatherAvailable = false;
                    continue;
                }

                if (!json_object_object_get_ex(parsed_json, "forecast", &forecast_obj) ||
                    !json_object_object_get_ex(forecast_obj, "forecastday", &forecastday_arr)) {
                    fprintf(stderr, "Error parsing forecast data.\n");
                    continue;
                }
                int days_available = json_object_array_length(forecastday_arr);
                int days_to_show = (days_available < FORECAST_DAYS) ? days_available : FORECAST_DAYS;

                for (int i = 0; i < days_to_show; i++) {
                    struct json_object *day_obj = json_object_array_get_idx(forecastday_arr, i);

                    struct json_object *date_obj, *day_data, *condition_obj, *icon_obj, *temperature_obj;
                    json_object_object_get_ex(day_obj, "date", &date_obj);
                    json_object_object_get_ex(day_obj, "day", &day_data);
                    json_object_object_get_ex(day_data, "condition", &condition_obj);
                    json_object_object_get_ex(condition_obj, "icon", &icon_obj);
                    json_object_object_get_ex(day_data, "avgtemp_c", &temperature_obj);



                    const char *date_str = json_object_get_string(date_obj);
                    const char *icon_path = json_object_get_string(icon_obj);
                    char *day_abbrev = get_weekday_abbrev(date_str);

                    strncpy(weatherData -> dailyWeekdays[i], day_abbrev, 8);
                    


                    if (temperature_obj) {
                        double temp_val = json_object_get_double(temperature_obj);
                        snprintf(weatherData->dailyTemperatures[i], sizeof(weatherData->dailyTemperatures[i]), "%d", (int)temp_val);
                        printf("Day: %s | Icon: https:%s| Temperature: %s \n ", day_abbrev, icon_path, temp_val);
                    } else {
                        printf("Temp object is null \n");
                        strncpy(weatherData->dailyTemperatures[i], "--", sizeof(weatherData->dailyTemperatures[i]));
                    }



                    free(day_abbrev);
                }
                printf("Should draw now!");
                weatherData -> weatherAvailable = true;
                weatherData -> weatherChanged = true;
                json_object_put(parsed_json); // free JSON object

            }
        }
        DEV_Delay_ms(10);

    }
    printf("Weather thread stopped. \n");

}

#define WEATHER_BOX_HEIGHT 32
#define WEATHER_BOX_HEIGHT 58
#define WEATHER_BOX_WIDTH 32

>>>>>>> Stashed changes
void ClockDraw(struct View* self) {
 
    ClockData* data = (ClockData*)self->data;
    if (!self || !self->data) {
        Debug("Data is invalid! \n");
        return;
    }
    //Paint_DrawString_EN(0, 0, "CLOCK", &Font16, WHITE, BLACK);
<<<<<<< Updated upstream
    Paint_DrawString_EN(0, 0, data -> formattedTime, &Font20, WHITE, BLACK);
    Paint_DrawString_EN(0, 20, data -> formattedDate, &Font16, WHITE, BLACK);
=======
    Paint_DrawString_EN(5, 5, data -> formattedTime, &Font24, WHITE, BLACK); 
    Paint_DrawString_EN(110, 9, data -> formattedDate, &Font16, WHITE, BLACK);


    if (data -> weatherData -> weatherAvailable) {
        printf("Drawing temperatures \n");
        data -> weatherData -> weatherChanged = false;
        

    
        for (int i = 0; i < FORECAST_DAYS; i++) {
            Paint_DrawString_EN(i * WEATHER_BOX_WIDTH, WEATHER_BOX_HEIGHT, data -> weatherData -> dailyWeekdays[i], &Font12, WHITE, BLACK);
            Paint_DrawRectangle(i * WEATHER_BOX_WIDTH, WEATHER_BOX_HEIGHT, (i+1) * WEATHER_BOX_WIDTH, WEATHER_BOX_HEIGHT + WEATHER_BOX_HEIGHT, BLACK, 1, false);
            Paint_DrawString_EN(i * WEATHER_BOX_WIDTH, WEATHER_BOX_HEIGHT + 12, data -> weatherData -> dailyTemperatures[i], &Font16, WHITE, BLACK);
        }

    }
>>>>>>> Stashed changes


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