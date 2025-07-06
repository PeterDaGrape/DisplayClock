# Intro
Display handler is designed to make managing the views easier, and has a couple of structures.

# TouchRegion
I know GUI programming is complicated and messy, so I tried to come up with a way of making regions, this could allow for items moving or changing, but is not used well currently. It has 2 funcitons, setRegion, and testRegion, both self explanatory.
For future reference, a better method would be to use callbacks, though that would require a competent programmer.

# View
This is a very important part, and where anyone using this shitty project will spend 90% of their time working with, so I tried to make it slightly less painful.

It has a pointer to an array of touchRegions.
numTouchRegions, ensures there is no overflow
data - a pointer to a structure defined by the view for holding state, like the time etc.
appName - The name of the app to show on the main menu
iconPath - If there is a bitmap in the rsc/pics for the view to show on the main menu, set this
closeView - Called by viewManager.switchView, used to cleanly stop any threads the view may employ
openView - Called by viewManager.switchView, used to start view dependent tasks
touch - when a touch occurs, this is called by the main loop, use it to handle touch interactions
draw - contains all instructions to reproduce the view
update - Used to update the state of a view



# ViewManager
This struct is designed to be simple and help coordinate the views. It has a switchView function, which takes a view pointer and calls the views openView and closeView functions.
- drawRequired - a variable to tell the main loop when a refresh is required
- forceFullRefresh - if a view requires that a full refresh is performed, the main loop will do one.
- currentView - Holds a pointer to the current view struct.