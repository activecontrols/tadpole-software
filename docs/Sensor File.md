## Documentation for Sensor Files

[PT Boards](controller/lib/pressure/)

[TC Boards](controller/lib/thermocouple/)


### Sensor.h file

Includes a class (that may derive from a sensor library object).
The class includes:
 - Private internal configuration variables
 - A constructor that sets these variables
 - A begin function that setups the sensor hardware
   - this is separate from the constructor so the constructors don't depend on hardware
 - Functions to read values from the sensors

Includes a namespace that contains the actual sensor objects, along with a begin function. The begin function is called by main.cpp and calls the begin functions on all of the sensor objects. Each sensor object must be declared as extern.

### Sensor.cpp file

Contains definitions for each of the functions described above.

### Library Files

Contains library files for interfacing with the actual sensor hardware.