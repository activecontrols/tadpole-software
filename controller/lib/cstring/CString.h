#ifndef CSTRING_H
#define CSTRING_H

#include <Arduino.h>

/*
 * CString.h
 *
 * Created on: 2024-08-10 by Vincent Palmerio
 * Maintained by Vincent Palmerio
 * Description: This file defines the CString and CStringPtr classes. Its goal is to make it super easy to concatenate strings and doubles
 * with char arrays. This is much better than using Arduino's String class, which is known to cause 
 * heap fragmentation after extended use. 
 */

template <size_t N>
class CString {
public:
    char str[N];
    int precision;
    int leftover;

    //Constructor to initialize the CString with an empty string and default precision
    CString() : precision(2) {
        clear();
    }

    //Set the precision for adding double values to the string
    void setPrecision(int p) {
        precision = p;
    }

    void clear() {
        str[0] = '\0';
    }

    //Overload the << operator to concatenate a char *
    CString& operator<<(const char* src) {
        size_t availableSpace = N - strlen(str) - 1;
        size_t srcLen = strlen(src);
        strncat(str, src, availableSpace);
        leftover = srcLen > availableSpace ? srcLen - availableSpace : 0;
        return *this;
    }

    //Overload the << operator to concatenate a double with specific precision (given by `precision` attribute)
    CString& operator<<(double value) {
        size_t availableSpace = N - strlen(str) - 1;
        char* end = str + strlen(str);
        size_t written = snprintf(end, availableSpace + 1, "%.*g", precision, value);
        leftover = written > availableSpace ? written - availableSpace : 0;
        return *this;
    }

    void print() const {
        Serial.println(str);
    }
};

//Non-template class for CString that takes a pointer to a CString
class CStringPtr {
public:

    //The buffer passed in to the constructor
    char* str;

    //size of the passed in char buffer
    size_t size;

    //Precision for double values when concatenated to string
    int precision;

    int leftover;

    //Constructor to initialize the CString and size
    CStringPtr(char* s, size_t n) : str(s), size(n), precision(2) {
        clear();
    }

    //Method to set the precision for double values
    void setPrecision(int p) {
        precision = p;
    }

    //Method to clear the C-string
    void clear() {
        str[0] = '\0';
    }

    //Overload the << operator to concatenate another C-string
    CStringPtr& operator<<(const char* src) {
        size_t availableSpace = size - strlen(str) - 1;
        size_t srcLen = strlen(src);
        strncat(str, src, availableSpace);
        leftover = srcLen > availableSpace ? srcLen - availableSpace : 0;
        return *this;
    }

    //Overload the << operator to concatenate a double with specific precision (precision changed by `setPrecision()`)
    CStringPtr& operator<<(double value)  {
        size_t availableSpace = size - strlen(str) - 1;
        char* end = str + strlen(str);
        size_t written = snprintf(end, availableSpace + 1, "%.*g", precision, value);
        leftover = written > availableSpace ? written - availableSpace : 0;
        return *this;
    }

    void print() const {
        Serial.println(str);
    }
};

#endif //CSTRING_H
