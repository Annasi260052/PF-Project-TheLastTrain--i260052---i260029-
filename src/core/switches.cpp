#include "switches.h"
#include "util.h"
#include "physics.h"

// Door lock mechanics, lifts, and cabinet verification without structures
#include <iostream>
using namespace std;
bool canUseLift1(int fromFloor, int toFloor){
    if (((fromFloor == 1 || fromFloor == 2) && (toFloor == 1 ||toFloor == 2)) && 
    (fromFloor != toFloor)){
        return true;
    }else return false;
}

bool canUseLift2(int fromFloor, int toFloor, bool lift2Operational, int emergencyTripsLeft){
bool validFloor((fromFloor == 2 || fromFloor==4 || fromFloor==5) && (toFloor==2 || toFloor==4 
    || toFloor==5));
bool diffFloor = (fromFloor!=toFloor);
if ((validFloor && diffFloor && lift2Operational)){
    return true;
}
else if ((emergencyTripsLeft > 0) && (validFloor) && (diffFloor)){
    return true;
}
else return false;
}

bool unlockFloor3Door(const char inventory[MAX_INVENTORY_CAPACITY], int inventoryCount){
    return hasItem(inventory,inventoryCount,ENT_MASTER_KEY);
}

bool installFuseControlPanel(char inventory[MAX_INVENTORY_CAPACITY], int& inventoryCount,
                             bool objectives[NUM_OBJECTIVES], int& score){
bool checkFuse = hasItem(inventory, inventoryCount, ENT_FUSE);
if (checkFuse == true){
if(removeItem(inventory, inventoryCount, ENT_FUSE)){
    if (!objectives[OBJ_FUSE_INSTALLED] ){
        objectives[OBJ_FUSE_INSTALLED] = true;
    applyScoreDelta(score, SCORE_INSTALL_FUSE);}
    if(objectives[OBJ_FUSE_INSTALLED] && objectives[OBJ_LIFT_PASS]){
        objectives[OBJ_LIFT2_ACTIVE] = true;
    }
    return true; } else return false;
} else return false;
          }

bool verifyCabinetCode(const char* enteredCode, const bool objectives[NUM_OBJECTIVES]){
bool clueObjectives = (objectives[OBJ_CLUE_4] && objectives[OBJ_CLUE_2] && objectives[OBJ_CLUE_7]); 
bool correctCode = stringEquals(enteredCode, "427");
if (clueObjectives && correctCode){
    return true;
} else return false;
}