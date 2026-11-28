#include "viewer_iface.h"
#include "util.h"
#include "level.h"
#include <iostream>
using namespace std;

// Terminal UI and dashboard rendering using primitive parameters
void renderDashboard(int playerFloor, int playerRoom, int energy, int time, int score,
                     const char inventory[MAX_INVENTORY_CAPACITY], int inventoryCount,
                     int emergencyTripsLeft)
 { // Room and values printing //
  if (playerFloor >= 0 && playerRoom >= 0)
   {cout << "F" << playerFloor + 1 << "-R" << playerRoom + 1 << endl;}
  else
   {cout << "Not selected yet" << endl;}
  cout << "| Energy Level: " << energy << endl;
  cout << "| Time: " << time << endl;
  cout << "| Score: " << score << endl;
  cout << "|- Emergency Trips left: " << emergencyTripsLeft << endl;
  // Inventory Printing //
  if (inventoryCount == 0)
   {cout << "No items\n";}
  else
   {for (int i = 0; i < inventoryCount; i++)
    {cout << i+1 << "." << " " << getEntityName(inventory[i]) << endl;}
   }
 }

 void renderRoomDetails(int floor, int room,
                       const char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                       int entityCount)
 {cout << "F" << (floor + 1) << "-R" << (room + 1) << endl;
  for (int i = 0; i < entityCount; i++)
   {cout << getEntityName(roomEntities[floor][room][i]) << endl;}
 }

 void renderAsciiBuilding(int playerFloor, int playerRoom)
  {for (int i = 4; i >= 0; i--)
   {for (int j = 0; j <= 3; j++)
     {if (i == playerFloor && j == playerRoom)
       cout << "[P] ";
      else
       cout << "F" << i << "-R" << j << " ";}
    cout << endl;}
  }

  void renderObjectivesChecklist(const bool objectives[NUM_OBJECTIVES])
   {const char* objectiveDisplay[14] = {"MASTER_KEY:", "CLUE_4:", "CLUE_2:", "CLUE_7:", "CABINET_UNLOCKED:",
                                  "TRAIN_KEY:", "FUSE_COLLECTED:", "FUSE_INSTALLED:", "LIFT_PASS:",
                                  "LIFT2_ACTIVE:", "PASSENGER_RESCUED:", "CAT_CARRIER:", "CAT_SECURED:",
                                  "TRAIN_PREPARED:"};                          
    for (int i = 0; i < 14; i++)
     {cout << objectiveDisplay[i];
      if (objectives[i])
       cout << " [X]" << endl;
      else
       cout << " -" << endl;
     }
    if (objectives[OBJ_MISSION_WON] == 1)
     cout << "Mission passed" << endl;
    else if (objectives[OBJ_MISSION_FAILED] == 1)
     cout << "Mission Failed" << endl;
    }