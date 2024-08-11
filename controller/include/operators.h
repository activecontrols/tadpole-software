#ifndef OPERATORS_H
#define OPERATORS_H

#include <Arduino.h>

//Add strings together using stream `<<` operator
String operator<<(String lhs, String rhs) {
    lhs += rhs;
    return lhs;
}

//Add float to a string together using stream `<<` operator
String operator<<(String &lhs, float rhs) {
    lhs += rhs;
    return lhs;
}

//Add int to a string together using stream `<<` operator
String operator<<(String &lhs, int rhs) {
    lhs += rhs;
    return lhs;
}

//Print with stream `<<` operator, taken from the ODriveUART.cpp file
template<class T> inline Print& operator <<(Print &obj,     T arg) { obj.print(arg);    return obj; }
template<>        inline Print& operator <<(Print &obj, float arg) { obj.print(arg, 4); return obj; }

#endif
