//
//  Created by Mark Coretsopoulos on 10/26/18.
//

#include <stdio.h>

#include "App.hpp"

int main(int argc, char **argv) {
    
    // argc, argv need to be passed to the app so that MinVR can parse command
    // line args to see which config files to load.
    App app(argc, argv);
    
    // Does not return until the program shuts down.
    app.run();
    
    return 0;
}
