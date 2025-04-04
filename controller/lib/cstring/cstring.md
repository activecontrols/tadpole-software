## CString.h

Created on: 2024-08-10 by Vincent Palmerio
Maintained by Vincent Palmerio
Description: This file defines the CString and CStringPtr classes, as well as the cstring 
namespace for helper functions. Its goal is to make it super easy to concatenate strings and
doubles with char arrays. This is much better than using Arduino's String class, which is known to cause 
heap fragmentation after extended use. 