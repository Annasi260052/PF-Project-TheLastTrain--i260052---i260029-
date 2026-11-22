#include "util.h"

// Primitive helper utilities, clamping, and screen control
// 22/11/2026 - Sunday //
#include <iostream>
#include <iomanip>
using namespace std;
// Setting value limits //
// 1. // 
int clampInt(int val, int minVal, int maxVal)
 {if (val > maxVal)
   {return maxVal;} 
  else if (val < minVal)
   {return minVal;}
  else
   {return val;}
 }
// 2. // 
 // Function made to check for spaces from the right side //
bool nonCharacterFromEnd(char a)
 {if (a == ' ' || a == '\n' || a == '\t' || a == '\r')
   return 1;
  else
   return 0;}
void trimWhitespace(char* str)
 {if (str == nullptr)
   {return;}
  int initial = 0;
  int corrected = 0;
  // Finding first real character here //
  for(;(nonCharacterFromEnd(str[initial]));)
   {initial++;}
  // Then sorting it out starting from real characters //
  for (;str[initial] != '\0';)
   {str[corrected] = str[initial];
    corrected++;
    initial++;}
  // Moving backwards from the end // 
  for (;(corrected > 0 && nonCharacterFromEnd(str[corrected - 1]));)
   {corrected--;}
  str[corrected] = '\0'; 
 }
// 3. //
bool stringEquals(const char* a, const char* b)
 {if (a == nullptr || b == nullptr)
   {return 0;}
  else
  {int i = 0;
   {for (; (a[i] != '\0' && b[i] != '\0'); i++)
      if (!(a[i] == b[i]))
       {return 0;}
    }
   return (a[i] == b[i]);
  }
 }
// 4. //
void printHeader(const char* title)
 {cout << '\n' << setw(26) << setfill('=') << '\n';
  cout << " " << title << endl;
  cout << setw(26) << endl << setfill(' ');}
// 5. //
void clearScreen()
 {cout << '\n' << setw(26) << setfill('-') << endl << setfill(' ');} 