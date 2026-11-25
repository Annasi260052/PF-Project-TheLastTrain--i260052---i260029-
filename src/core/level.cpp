#include "level.h"
#include "util.h"
#include "config.h"
#include <iostream>
#include <fstream>
using namespace std;

// Modular implementation using primitive arrays for map parsing and verification
// 1. //
// This is comparing the symbols from state.h get a name for the item //
const char* getEntityName(char entitySymbol)
 {if (entitySymbol == ENT_NONE)
   {return "Vacant";}
  else if (entitySymbol == ENT_MASTER_KEY)
   {return "Master Key";}
  else if (entitySymbol == ENT_ENERGY_PACK)
   {return "Energy Pack";}
  else if (entitySymbol == ENT_EMERGENCY_TOOLKIT)
   {return "Emergency Toolkit";}
  else if (entitySymbol == ENT_CLUE_4)
   {return "Clue digit 4";}
  else if (entitySymbol == ENT_FUSE)
   {return "Fuse";}
  else if (entitySymbol == ENT_CAT)
   {return "Cat";}
  else if (entitySymbol == ENT_CONTROL_PANEL)
   {return "Control Panel";}
  else if (entitySymbol == ENT_LIFT_PASS)
   {return "Lift Pass";}
  else if (entitySymbol == ENT_PASSENGER)
   {return "Stranded Passenger";}
  else if (entitySymbol == ENT_MEDICAL_KIT)
   {return "Medical Kit";}
  else if (entitySymbol == ENT_EMERGENCY_CARD)
   {return "Emergency Access Card";}
  else if (entitySymbol == ENT_CLUE_2)
   {return "Clue digit 2";}
  else if (entitySymbol == ENT_CLUE_7)
   {return "Clue digit 7";}
  else if (entitySymbol == ENT_CABINET_TRAIN_KEY)
   {return "Cabinet / Train key";}
  else if (entitySymbol == ENT_CAT_CARRIER)
   {return "Cat Carrier";}
  else if (entitySymbol == ENT_OBSERVATION_TOKEN)
   {return "Observation Token";}
  else
   {return "Absent";}
}
// 2. //
bool validateStartingRoom(int startFloor, int startRoom, const char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR - 1])
 {bool statusRoom = true;
  if (startFloor < 0 || startFloor >= NUM_FLOORS)
   {statusRoom = false;}
  else if (startRoom < 0 || startRoom >= ROOMS_PER_FLOOR)
   {statusRoom = false;}
  else if (horizontalConn[startFloor][0] == CONN_NONE)
   {statusRoom = false;}
  return statusRoom;
 }
 // 3. //
 // Loading the map //
 bool loadBuildingMap(const char* filename,
                     char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR - 1],
                     char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                     int entityCounts[NUM_FLOORS][ROOMS_PER_FLOOR])
 {for (int i = 0; i < NUM_FLOORS; i++)
   {for (int j = 0; j < ROOMS_PER_FLOOR; j++)
     {entityCounts[i][j] = 0;
      for (int k = 0; k < MAX_ENTITIES_PER_ROOM; k++)
        {roomEntities[i][j][k] = ENT_NONE;}
      if (j < ROOMS_PER_FLOOR - 1)
        {horizontalConn[i][j] = CONN_NONE;}
     }
   }
 ifstream file(filename);
 if (!file.is_open())
  return false;
 // Lines are at max 70 columns long //
 char line[MAX_LINE_LEN];
 int floorsMoved = 0;
 while (file.getline(line, MAX_LINE_LEN))
  {int startingPosition = -1;
   for (int i = 0; (line[i] != '\0' && line[i + 1] != '\0' && line[i + 2] != '\0' && line[i + 3] != '\0' && line[i + 4] != '\0'); i++)
    {if (line[i] == 'F' && line[i + 1] >= '1' && line[i + 1] <= '5' && line[i + 2] == '-' && line[i + 3] == 'R' && line[i + 4] == '1')
      {startingPosition = i;
       break;}
    }
   if (startingPosition == -1)
    {continue;}
   // horizontalConn is from 0 to 4 //
   int floorIndex = line[startingPosition + 1] - '1';
   int connIndex = 0;
   for (int j = startingPosition + 5; line[j] != '\0' && line[j+1] != '\0' && line[j+2] != '\0' && connIndex < ROOMS_PER_FLOOR - 1; j++)
    {if (line[j] == '-' && line[j+1] == 'R' && (line[j+2] == '2' || line[j+2] == '3' || line[j+2] == '4'))
      {char symbol = line[j-4];
       if (symbol == '1' && line[j-5] == 'L')
        {horizontalConn[floorIndex][connIndex] = CONN_DOOR;}
       else if (symbol == 'D')
        {horizontalConn[floorIndex][connIndex] = CONN_DOOR;}
       else if (symbol == 'W')
        {horizontalConn[floorIndex][connIndex] = CONN_WINDOW;}
       else if (symbol == '#')
        {horizontalConn[floorIndex][connIndex] = CONN_SOLID_WALL;}
       else if (symbol == 'L')
        {horizontalConn[floorIndex][connIndex] = CONN_LOCKED_DOOR;}
       connIndex++;
      }
    }
  char signInLine[MAX_LINE_LEN];
  if (file.getline(signInLine, MAX_LINE_LEN))
   {int roomIndex = 0;
    for (int k = startingPosition; (signInLine[k] != '\0' && roomIndex < ROOMS_PER_FLOOR); k++)
     {if (signInLine[k] == '|')
       {roomIndex++;}
      else if (signInLine[k] != ' ' && entityCounts[floorIndex][roomIndex] < MAX_ENTITIES_PER_ROOM)
       {roomEntities[floorIndex][roomIndex][entityCounts[floorIndex][roomIndex]] = signInLine[k];
        entityCounts[floorIndex][roomIndex]++;}
     }
   }
 floorsMoved++;
 }
 file.close();
 return (floorsMoved == NUM_FLOORS);
 }
 //4.//
 // Loading in the first initial states //
 int conversion(const char* characters)
  {int result, sign, i;
   result = 0;
   sign = 1; // Positive //
   i = 0;
   if (characters[0] == '-')
    {sign = -1;
     i = 1;}
   for (;characters[i] >= '0' && characters[i] <= '9';i++)
    {result = result * 10 + (characters[i] - '0');}
   return (sign * result);
  }
 bool loadInitialState(const char* filename,
                      int& playerFloor, int& playerRoom,
                      int& energy, int& time, int& score,
                      char inventory[MAX_INVENTORY_CAPACITY], int& inventoryCount,
                      bool objectives[NUM_OBJECTIVES], int& emergencyTripsLeft)
 {playerFloor = -1; // array has value at 0 and we have to start from behind it //
  playerRoom = -1;
  energy = INITIAL_ENERGY;
  time = INITIAL_TIME;
  score = INITIAL_SCORE;
  inventoryCount = 0;
  emergencyTripsLeft = 3;
  for (int i = 0; i < MAX_INVENTORY_CAPACITY; i++) // inventory items in parameter //
   {inventory[i] = ENT_NONE;}
  for (int i = 0; i < NUM_OBJECTIVES; i++) // objectives completion as bool in parameter //
   {objectives[i] = false;}
  ifstream file(filename);
  if (!file.is_open())
   {return false;}
  char keysAndObjects[MAX_LINE_LEN];
  char valueOfKO[MAX_LINE_LEN];
  const char* objectiveNames[7] = {"PASSENGER_RESCUED:", "CAT_RESCUED:", "CABINET_UNLOCKED:",
    "TRAIN_KEY_COLLECTED:", "FUSE_INSTALLED:", "LIFT_PASS_COLLECTED:",
    "TRAIN_PREPARED:"};
  int objectivesIndex[7] = {OBJ_PASSENGER_RESCUED, OBJ_CAT_SECURED, OBJ_CABINET_UNLOCKED,
    OBJ_TRAIN_KEY, OBJ_FUSE_INSTALLED, OBJ_LIFT_PASS, OBJ_TRAIN_PREPARED};

  while (file >> keysAndObjects >> valueOfKO)
   { // For Room first from initial-state.txt //
    if ((stringEquals(keysAndObjects, "START_ROOM:")))
     {if (valueOfKO[0] == 'F' && valueOfKO[1] >= '1' && valueOfKO[1] <= '5' &&
          valueOfKO[2] == '-' && valueOfKO[3] == 'R' &&
          valueOfKO[4] >= '1' && valueOfKO[4] <= '4')
        {playerRoom = valueOfKO[4] - '1';
         playerFloor = valueOfKO[1] - '1';}
     }
    // Initial energy - score //
    // 1. // 
    else if(stringEquals(keysAndObjects, "INITIAL_ENERGY:"))
     {energy = conversion(valueOfKO);}
    else if(stringEquals(keysAndObjects, "INITIAL_TIME:"))
     {time = conversion(valueOfKO);}
    else if(stringEquals(keysAndObjects, "INITIAL_SCORE:"))
     {score = conversion(valueOfKO);}
    // 2. //
    // Inventory and cards collected //
    int iAndCcase = 0;
    if (stringEquals(keysAndObjects, "INITIAL_INVENTORY:"))
     {iAndCcase = 2;}
    else if (stringEquals(keysAndObjects, "CLUES_COLLECTED:"))
     {iAndCcase = 3;}
    if (iAndCcase == 2 && (!stringEquals(valueOfKO, "NONE")))
     {for (int i = 0; valueOfKO[i] != '\0'; i++)
       {if (valueOfKO[i] != ',' && inventoryCount < MAX_INVENTORY_CAPACITY)
         {inventory[inventoryCount] = valueOfKO[i];
          inventoryCount++;}
       }
     }
    else if (iAndCcase == 3 && (!stringEquals(valueOfKO, "NONE")))
     {for (int i = 0; valueOfKO[i] != '\0'; i++)
       {if (valueOfKO[i] == ENT_CLUE_4)
         objectives[OBJ_CLUE_4] = true;
        else if (valueOfKO[i] == ENT_CLUE_2)
         objectives[OBJ_CLUE_2] = true;
        else if (valueOfKO[i] == ENT_CLUE_7)
         objectives[OBJ_CLUE_7] = true;
       }
     }
    // For the objectives //
    for (int i = 0; i < 7; i++)
     {if (stringEquals(keysAndObjects, objectiveNames[i]))
       {objectives[objectivesIndex[i]] = stringEquals(valueOfKO, "YES");}
     }
  }
 file.close();
 return true;
}