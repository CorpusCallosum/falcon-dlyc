/*
  
*/

#ifndef Pattern_h
#define Pattern_h

#include "Arduino.h"

class Pattern
{
  public:
    Pattern();
    //void set(uint8_t pattern);
    //bool comparePatternTo(uint8_t pattern [5][5]);
    String _soundFileName;
    String _pattern[5];
    
  private:
   // String _soundFileName;
   // uint8_t _pattern[5][5];
};

#endif
