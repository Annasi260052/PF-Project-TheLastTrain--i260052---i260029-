
#include "physics.h"
#include "util.h"
#include "state.h"
// Movement validation and resource adjustments without structures
bool hasItem(const char inventory[MAX_INVENTORY_CAPACITY], int inventoryCount, char itemSymbol){
for (int i = 0; i < inventoryCount;i++){
    if ( inventory[i] == itemSymbol )
    return true;
} return false;
}
bool addItem(char inventory[MAX_INVENTORY_CAPACITY], int& inventoryCount, char itemSymbol){
    if (inventoryCount == MAX_INVENTORY_CAPACITY){
        return false;
    } else {
        inventory[inventoryCount] = itemSymbol;
        inventoryCount++;
        return true;
}}
bool removeItem(char inventory[MAX_INVENTORY_CAPACITY], int& inventoryCount, char itemSymbol){
    for (int i = 0; i < inventoryCount; i ++){
        if (inventory[i] == itemSymbol){
                        for (int j = i; j < inventoryCount - 1; j++) {
                            inventory[j] = inventory[j + 1];
            }
            inventoryCount--;
            return true;
        } 
    }
    return false;
}
void applyResourceDelta(int& energy, int& time, int deltaEnergy, int deltaTime){
    energy += deltaEnergy;
    time += deltaTime;
    energy = clampInt(energy,0,MAX_ENERGY);
    time = clampInt( time,0,MAX_TIME );
}
void applyScoreDelta(int& score, int deltaScore){
    score += deltaScore;   
}
bool isDirectlyConnected(int fromF, int fromR, int toF, int toR,
                         const char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR - 1],
                         char& outConnType){
                 
    if (fromF < 1 || fromF > NUM_FLOORS || toF < 1 || toF > NUM_FLOORS ||
        fromR < 1 || fromR > ROOMS_PER_FLOOR || toR < 1 || toR > ROOMS_PER_FLOOR) {
        return false;
    }

    // Same floor: read the stored link between neighbouring rooms
    if (fromF == toF) {
        if (toR - fromR != 1 && toR - fromR != -1) return false;
        int smaller = (fromR < toR) ? fromR : toR;
        char cell = horizontalConn[fromF - 1][smaller - 1];
        if (cell == CONN_NONE) return false;
        outConnType = cell;
        return true;
    }

    // Different floors: find the lower and higher floor so both directions work
    int low  = (fromF < toF) ? fromF : toF;
    int high = (fromF < toF) ? toF : fromF;

    // Stairs: R1 to R1 between floors 1-2, 2-3, 4-5
    if (fromR == 1 && toR == 1 &&
        ((low == 1 && high == 2) || (low == 2 && high == 3) || (low == 4 && high == 5))) {
        outConnType = CONN_STAIRS;
        return true;
    }

    // Lift 1: floors 1-2, rooms 2 or 3
    if (low == 1 && high == 2 &&
        (fromR == 2 || fromR == 3) && (toR == 2 || toR == 3)) {
        outConnType = CONN_LIFT1;
        return true;
    }

    // Lift 2: R4 on floors 2, 4 and 5
    if (fromR == 4 && toR == 4 &&
        (fromF == 2 || fromF == 4 || fromF == 5) &&
        (toF == 2 || toF == 4 || toF == 5)) {
        outConnType = CONN_LIFT2;
        return true;
    }

    return false;
}
