//
// Created by Ishan Goel on 6/11/24.
//

#ifndef TADPOLE_SOFTWARE_SDCARD_H
#define TADPOLE_SOFTWARE_SDCARD_H

#include <SD.h>

class SDCard {
public:
    static boolean begin();
    static File open(char filename[], char mode);
private:
    // used by comms
    static void ls();
//    static void cat();
};


#endif //TADPOLE_SOFTWARE_SDCARD_H
