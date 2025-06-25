#include "DisplayHandler.h"
#include <stdio.h>
#include <stdlib.h>     //exit()
#include "Debug.h"

void TouchRegion_setRegion(TouchRegion* self, int x, int y, int xSize, int ySize) {
    self->xStart = x;
    self->yStart = y;
    self->xStop = x + xSize;
    self->yStop = y + ySize;
}

bool TouchRegion_testRegion(TouchRegion* self, int x, int y) {
    return (x >= self->xStart && x <= self->xStop &&
            y >= self->yStart && y <= self->yStop);
}

void ViewManagerSwitchView(struct ViewManager* self, struct View* view) {
    if (!self) {
        printf("ViewManagerSwitchView: self is NULL!\n");
        return;
    }
    if (!view) {
        printf("ViewManagerSwitchView: view is NULL!\n");
        return;
    }
    
    if (self->currentView && self->currentView->closeView) {
        self -> currentView -> closeView(self -> currentView);
    }

    self -> currentView = view;
    if (self -> currentView -> openView) {
        self -> currentView -> openView(self -> currentView);
    }
    self -> drawRequired = true;

    printf("View switched \n");
}


ViewManager viewManager = {
    .drawRequired = false,
    .forceFullRefresh = false,
    .currentView = NULL,
    .switchView = ViewManagerSwitchView
};




