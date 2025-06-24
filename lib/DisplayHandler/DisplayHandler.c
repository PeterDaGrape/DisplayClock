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
    self -> currentView = view;
    self -> drawRequired = true;
    printf("Draw required %i \n", self -> drawRequired);

}


ViewManager viewManager = {
    .drawRequired = false,
    .forceFullRefresh = false,
    .currentView = NULL,
    .switchView = ViewManagerSwitchView
};




