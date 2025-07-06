#include <stdio.h>
#include <stdlib.h>     //exit()
#include <signal.h>     //signal()
#include "EPD_2in13_V3.h"
#include "GUI_Paint.h"
#include "GT1151.h"
#include "GUI_BMPfile.h"
#include <stdbool.h>

#include <curl.h>
#include <json-c/json.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"

struct Memory {
    char *response;
    size_t size;
};
unsigned char *convert_to_grayscale(unsigned char *rgb, int width, int height, int channels) {
    unsigned char *gray = malloc(width * height);
    if (!gray) return NULL;

    for (int i = 0; i < width * height; i++) {
        int r = rgb[i * channels + 0];
        int g = rgb[i * channels + 1];
        int b = rgb[i * channels + 2];
        if (0.3 * r + 0.59 * g + 0.11 * b > 0.5) {
            gray[i] = 0;
        } else {
            gray[i] = 1;
        }
    }

    return gray;
}

// Buffer to collect BMP output
struct BMPBuffer {
    unsigned char *data;
    size_t size;
};

// stbi_write callback
void bmp_write_callback(void *context, void *data, int size) {
    struct BMPBuffer *buf = (struct BMPBuffer *)context;
    unsigned char *ptr = realloc(buf->data, buf->size + size);
    if (!ptr) return;

    memcpy(ptr + buf->size, data, size);
    buf->data = ptr;
    buf->size += size;
}


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

//Affects the epd driver
void stopHandler(int signo)
{
    //System Exit
    printf("\r\nHandler:exit\r\n");
	EPD_2in13_V3_Sleep();
	DEV_Delay_ms(2000);

    DEV_ModuleExit();
    exit(0);
}
extern int IIC_Address;

int main() {


    //Setup hardware
    IIC_Address = 0x14;
    signal(SIGINT, stopHandler);
    DEV_ModuleInit();


    EPD_2in13_V3_Init(EPD_2IN13_V3_FULL);
    EPD_2in13_V3_Clear();

	DEV_Delay_ms(100);


    //Create a new image cache
    UBYTE *image;
    UWORD Imagesize = ((EPD_2in13_V3_WIDTH % 8 == 0)? (EPD_2in13_V3_WIDTH / 8 ): (EPD_2in13_V3_WIDTH / 8 + 1)) * EPD_2in13_V3_HEIGHT;

    if((image = (UBYTE *)malloc(Imagesize)) == NULL) {
        printf("Failed to apply for black memory...\r\n");
        return -1;
    }
    

    Paint_NewImage(image, EPD_2in13_V3_WIDTH, EPD_2in13_V3_HEIGHT, ROTATE_270, WHITE);
    Paint_SelectImage(image);
    Paint_SetMirroring(MIRROR_ORIGIN);
    Paint_Clear(WHITE);

    printf("Starting curl \n");

    //draw here
    CURL *curl;
    CURLcode res;
    struct Memory chunk;
    chunk.response = malloc(1);  // Initialize memory
    curl_global_init(CURL_GLOBAL_DEFAULT);
    char apiURL[256];
    sprintf(apiURL, "https://openweathermap.org/img/wn/10d@2x.png");


    // Initialize libcurl
    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, apiURL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ResponseCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);         // 10 seconds max for the whole request
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);   // 5 seconds max to connect
    printf("Completed setup of curl \n");
    chunk.size = 0;             // No data yet
    // Perform the request
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    printf("completed download \n");
    // Check for errors
    if (res != CURLE_OK) {
        printf("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        return 1;
    }

    if (!chunk.response) {
        printf("Download failed\n");
        return 1;
    }

    // Step 2: Decode PNG
    int w, h, channels;
    unsigned char *pixels = stbi_load_from_memory(chunk.response, chunk.size, &w, &h, &channels, 0);
    free(chunk.response); // no longer needed
    
    // Step 3: Resize
    int new_w = 128; 
    int new_h = 128;
    unsigned char *resized = malloc(new_w * new_h * 3);
    stbir_resize_uint8(pixels, w, h, 0, resized, new_w, new_h, 0, 3);
    stbi_image_free(pixels);

	EPD_2in13_V3_Display(image);
	EPD_2in13_V3_Init(EPD_2IN13_V3_PART);
	EPD_2in13_V3_Display_Partial_Wait(image);


    return 0;
}