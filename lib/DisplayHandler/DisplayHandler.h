#ifndef __DISPLAY_HANDLER
#define __DISPLAY_HANDLER

#define VIEW_MAIN_MENU 0

#include <stdbool.h>


typedef struct TouchRegion {
    int xStart;
    int xStop;
    int yStart;
    int yStop;

    void (*setRegion)(struct TouchRegion* self, int x, int y, int xSize, int ySize);
    bool (*testRegion)(struct TouchRegion* self, int x, int y);
} TouchRegion;

void TouchRegion_setRegion(TouchRegion* self, int x, int y, int xSize, int ySize);
bool TouchRegion_testRegion(TouchRegion* self, int x, int y);

typedef struct View {

    TouchRegion* touchRegions;
    int numTouchRegions;

    void* data;

    char appName[100];

    char iconPath [100];

    void (*closeView)(struct View* self);

    void (*openView)(struct View* self);

    void (*touch)(struct View* self, int x, int y);
    
    void (*draw)(struct View* self);
    void (*update)(struct View* self);

} View;

typedef struct ViewManager {

    bool drawRequired;
    bool forceFullRefresh;

    View* currentView;
    void (*switchView)(struct ViewManager* self, View* view);

} ViewManager;
extern ViewManager viewManager;

#endif