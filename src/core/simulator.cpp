#include "simulator.h"
#include "physics.h"
#include "switches.h"
#include "logger.h"
#include "viewer_iface.h"
#include "level.h"
#include "util.h"

#include <iostream>
#include <iomanip>
#include <limits>

using namespace std;

// checkEndConditions: true when the session must stop (win or any resource at 0)
bool checkEndConditions(int energy, int time, const bool objectives[NUM_OBJECTIVES]) {
    if (energy <= 0 || time <= 0 || objectives[OBJ_MISSION_WON]) {
        return true;
    }
    return false;
}
// handleMoveCommand: validate the move, then charge the cost and move the playe
bool handleMoveCommand(int targetFloor, int targetRoom,
                       int& currentFloor, int& currentRoom,
                       int& energy, int& time,
                       const char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR - 1],
                       const char inventory[MAX_INVENTORY_CAPACITY], int inventoryCount,
                       int& emergencyTripsLeft, bool lift2Operational) {

    char link = CONN_NONE;

    // Check 1: is there a link between the two rooms?
    bool linked = isDirectlyConnected(currentFloor, currentRoom,
                                      targetFloor, targetRoom,
                                      horizontalConn, link);
    if (linked == false) {
        cout << "There is no direct route to that room.\n";
        return false;
    }

    // Check 2: is that kind of link passable right now?
    bool allowed = canTraverse(targetFloor, targetRoom, link,
                               inventory, inventoryCount,
                               emergencyTripsLeft, lift2Operational);
    if (allowed == false) {
        switch (link) {
            case CONN_SOLID_WALL:
                cout << "A solid wall blocks the way.\n";
                break;
            case CONN_LOCKED_DOOR:
                cout << "The door is locked. You need the Master Key.\n";
                break;
            case CONN_LIFT2:
                cout << "Lift 2 is not available.\n";
                break;
            default:
                cout << "You cannot go that way.\n";
                break;
        }
        return false;
    }

    // Nothing has been charged or changed so far. Choose the cost.
    int costEnergy = COST_MOVE_ORDINARY_ENERGY;
    int costTime   = COST_MOVE_ORDINARY_TIME;

    switch (link) {
        case CONN_WINDOW:
            costEnergy = COST_MOVE_WINDOW_ENERGY;
            costTime   = COST_MOVE_WINDOW_TIME;
            break;
        case CONN_LIFT1:
        case CONN_LIFT2:
            costEnergy = COST_MOVE_LIFT_ENERGY;
            costTime   = COST_MOVE_LIFT_TIME;
            break;
        default:
            break; // doors, locked door and stairs keep the ordinary cost
    }

    applyResourceDelta(energy, time, costEnergy, costTime);

   // Lift 2 before repair uses up one emergency trip
    bool usedEmergencyTrip = false;
    if (link == CONN_LIFT2 && lift2Operational == false) {
        emergencyTripsLeft--;
        usedEmergencyTrip = true;
    }

    currentFloor = targetFloor;
    currentRoom  = targetRoom;

    cout << "You moved to F" << currentFloor << "-R" << currentRoom;
    if (usedEmergencyTrip) {
        cout << " (emergency trips left: " << emergencyTripsLeft << ")";
    }
    cout << ".\n";

    return true;
}

// handleInspectCommand: costs Energy/Time, lists the room, reveals clue digits

bool handleInspectCommand(int currentFloor, int currentRoom,
                          int& energy, int& time,
                          const char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                          int entityCount,
                          bool objectives[NUM_OBJECTIVES], int& score) {

    applyResourceDelta(energy, time, COST_INSPECT_ENERGY, COST_INSPECT_TIME);

    cout << "You search the room carefully...\n";
    renderRoomDetails(currentFloor, currentRoom, roomEntities, entityCount);

    int f = currentFloor - 1;
    int r = currentRoom - 1;

    for (int i = 0; i < entityCount; i++) {
        char e = roomEntities[f][r][i];

        if (e == ENT_CLUE_4 && objectives[OBJ_CLUE_4] == false) {
            objectives[OBJ_CLUE_4] = true;
            applyScoreDelta(score, SCORE_CLUE_FOUND);
            cout << "Clue found: the first digit is 4.\n";
        } else if (e == ENT_CLUE_2 && objectives[OBJ_CLUE_2] == false) {
            objectives[OBJ_CLUE_2] = true;
            applyScoreDelta(score, SCORE_CLUE_FOUND);
            cout << "Clue found: the second digit is 2.\n";
        } else if (e == ENT_CLUE_7 && objectives[OBJ_CLUE_7] == false) {
            objectives[OBJ_CLUE_7] = true;
            applyScoreDelta(score, SCORE_CLUE_FOUND);
            cout << "Clue found: the third digit is 7.\n";
        }
    }

    return true;
}

// ---------------------------------------------------------------------------
// handleCollectCommand: pick an item up from the current room
// ---------------------------------------------------------------------------
bool handleCollectCommand(char itemSymbol, int currentFloor, int currentRoom,
                          int& energy, int& time, int& score,
                          char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                          int& entityCount,
                          char inventory[MAX_INVENTORY_CAPACITY], int& inventoryCount,
                          bool objectives[NUM_OBJECTIVES]) {

    int f = currentFloor - 1;
    int r = currentRoom - 1;

    // Find the item in this room
    int index = -1;
    for (int i = 0; i < entityCount; i++) {
        if (roomEntities[f][r][i] == itemSymbol) {
            index = i;
            break;
        }
    }
    if (index == -1) {
        cout << "There is no such item here.\n";
        return false;
    }

    // Things that cannot be picked up
    if (itemSymbol == ENT_CLUE_4 || itemSymbol == ENT_CLUE_2 || itemSymbol == ENT_CLUE_7) {
        cout << "Clue digits are found by inspecting the room.\n";
        return false;
    }
    if (itemSymbol == ENT_PASSENGER) {
        cout << "The passenger needs the Medical Kit, not a pickup.\n";
        return false;
    }
    if (itemSymbol == ENT_CAT) {
        cout << "The cat needs the Cat Carrier, not a pickup.\n";
        return false;
    }
    if (itemSymbol == ENT_CONTROL_PANEL) {
        cout << "The control panel is fixed in place.\n";
        return false;
    }
    if (itemSymbol == ENT_CABINET_TRAIN_KEY && objectives[OBJ_CABINET_UNLOCKED] == false) {
        cout << "The cabinet is locked. Unlock it with the code first.\n";
        return false;
    }

    bool consumable = (itemSymbol == ENT_ENERGY_PACK ||
                       itemSymbol == ENT_EMERGENCY_TOOLKIT ||
                       itemSymbol == ENT_OBSERVATION_TOKEN);

    if (consumable) {
        // Applied immediately, never stored in the inventory
        if (itemSymbol == ENT_ENERGY_PACK) {
            applyResourceDelta(energy, time, BONUS_ENERGY_PACK, 0);
            cout << "Energy Pack used: +" << BONUS_ENERGY_PACK << " Energy.\n";
        } else if (itemSymbol == ENT_EMERGENCY_TOOLKIT) {
            applyResourceDelta(energy, time, BONUS_EMERGENCY_TOOLKIT, 0);
            cout << "Emergency Toolkit used: +" << BONUS_EMERGENCY_TOOLKIT << " Energy.\n";
        } else {
            applyResourceDelta(energy, time, 0, BONUS_OBSERVATION_TOKEN);
            cout << "Observation Token used: +" << BONUS_OBSERVATION_TOKEN << " Time.\n";
        }
    } else {
        // Normal items need free inventory space
        if (inventoryCount >= MAX_INVENTORY_CAPACITY) {
            cout << "Your inventory is full.\n";
            return false;
        }

        addItem(inventory, inventoryCount, itemSymbol);
        applyResourceDelta(energy, time, COST_ITEM_PICKUP_ENERGY, COST_ITEM_PICKUP_TIME);

        switch (itemSymbol) {
            case ENT_MASTER_KEY:
                objectives[OBJ_MASTER_KEY] = true;
                applyScoreDelta(score, SCORE_KEY_COLLECT);
                break;
            case ENT_FUSE:
                objectives[OBJ_FUSE_COLLECTED] = true;
                break;
            case ENT_LIFT_PASS:
                objectives[OBJ_LIFT_PASS] = true;
                applyScoreDelta(score, SCORE_LIFT_PASS);
                if (objectives[OBJ_FUSE_INSTALLED]) {
                    objectives[OBJ_LIFT2_ACTIVE] = true;
                }
                break;
            case ENT_CAT_CARRIER:
                objectives[OBJ_CAT_CARRIER] = true;
                break;
            case ENT_CABINET_TRAIN_KEY:
                objectives[OBJ_TRAIN_KEY] = true;
                applyScoreDelta(score, SCORE_TRAIN_KEY);
                break;
            default:
                break; // Medical Kit and Emergency Access Card need no extra flag
        }

        cout << "You picked up " << itemSymbol << ".\n";
    }

    // Remove the item from the room by shifting the rest left
    for (int j = index; j < entityCount - 1; j++) {
        roomEntities[f][r][j] = roomEntities[f][r][j + 1];
    }
    entityCount--;

    return true;
}


// handleUseCommand: use an inventory item where it matters

bool handleUseCommand(char itemSymbol, int currentFloor, int currentRoom,
                      int& energy, int& time, int& score,
                      char inventory[MAX_INVENTORY_CAPACITY], int& inventoryCount,
                      bool objectives[NUM_OBJECTIVES]) {

    if (hasItem(inventory, inventoryCount, itemSymbol) == false) {
        cout << "You do not have that item.\n";
        return false;
    }

    bool done = false;

    if (itemSymbol == ENT_FUSE) {
        if (currentFloor == 3 && currentRoom == 4) {
            done = installFuseControlPanel(inventory, inventoryCount, objectives, score);
            if (done) {
                cout << "Fuse installed in the Control Panel.\n";
            }
        } else {
            cout << "The Fuse must be installed at the Control Panel in F3-R4.\n";
        }
    } else if (itemSymbol == ENT_MEDICAL_KIT) {
        if (currentFloor == 4 && currentRoom == 1) {
            if (objectives[OBJ_PASSENGER_RESCUED]) {
                cout << "The passenger has already been rescued.\n";
            } else {
                objectives[OBJ_PASSENGER_RESCUED] = true;
                applyScoreDelta(score, SCORE_RESCUE_PASSENGER);
                removeItem(inventory, inventoryCount, ENT_MEDICAL_KIT);
                cout << "You treat the injured passenger. Passenger rescued!\n";
                done = true;
            }
        } else {
            cout << "The Medical Kit is used on the passenger in F4-R1.\n";
        }
    } else if (itemSymbol == ENT_CAT_CARRIER) {
        if (currentFloor == 3 && currentRoom == 3) {
            if (objectives[OBJ_CAT_SECURED]) {
                cout << "The cat is already secured.\n";
            } else {
                objectives[OBJ_CAT_SECURED] = true;
                applyScoreDelta(score, SCORE_SECURE_CAT);
                removeItem(inventory, inventoryCount, ENT_CAT_CARRIER);
                cout << "You secure the cat in the carrier. Cat secured!\n";
                done = true;
            }
        } else {
            cout << "The Cat Carrier is used on the cat in F3-R3.\n";
        }
    } else if (itemSymbol == ENT_CABINET_TRAIN_KEY) {
        if (currentFloor == 5 && currentRoom == 2) {
            if (objectives[OBJ_PASSENGER_RESCUED] == false) {
                cout << "You cannot leave without rescuing the passenger.\n";
            } else if (objectives[OBJ_CAT_SECURED] == false) {
                cout << "You cannot leave without securing the cat.\n";
            } else {
                objectives[OBJ_TRAIN_PREPARED] = true;
                applyScoreDelta(score, SCORE_PREPARE_TRAIN);
                objectives[OBJ_MISSION_WON] = true;
                applyScoreDelta(score, SCORE_MISSION_VICTORY);
                cout << "The Train Key starts the engine. Train prepared and boarded!\n";
                done = true;
            }
        } else {
            cout << "The Train Key is used at the Train Boarding Platform in F5-R2.\n";
        }
    } else if (itemSymbol == ENT_MASTER_KEY) {
        cout << "The Master Key unlocks the F3-R1 / F3-R2 door when you walk through it.\n";
    } else {
        cout << "That item cannot be used here.\n";
    }

    if (done == false) {
        return false; // rejected actions cost nothing
    }

    applyResourceDelta(energy, time, COST_ITEM_USE_ENERGY, COST_ITEM_USE_TIME);
    return true;
}

// handleUnlockCabinetCommand: enter the 3-digit code at the cabinet in F5-R1
bool handleUnlockCabinetCommand(const char* code, int currentFloor, int currentRoom,
                                bool objectives[NUM_OBJECTIVES], int& score,
                                char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                                int& entityCount) {

    if (currentFloor != 5 || currentRoom != 1) {
        cout << "The cabinet is in F5-R1.\n";
        return false;
    }

    if (objectives[OBJ_CABINET_UNLOCKED]) {
        cout << "The cabinet is already unlocked.\n";
        return false;
    }

    // Make sure the cabinet is really in this room
    bool cabinetHere = false;
    for (int i = 0; i < entityCount; i++) {
        if (roomEntities[currentFloor - 1][currentRoom - 1][i] == ENT_CABINET_TRAIN_KEY) {
            cabinetHere = true;
        }
    }
    if (cabinetHere == false) {
        cout << "There is no cabinet here.\n";
        return false;
    }

    if (verifyCabinetCode(code, objectives) == false) {
        cout << "The cabinet stays locked. Wrong code, or you have not found every clue digit.\n";
        return false;
    }

    objectives[OBJ_CABINET_UNLOCKED] = true;
    applyScoreDelta(score, SCORE_CABINET_UNLOCK);
    cout << "Click! The cabinet opens. Collect the Train Key (^).\n";
    return true;
}

// runSimulation: the main game loop

void runSimulation(int& playerFloor, int& playerRoom,
                   int& energy, int& time, int& score,
                   char inventory[MAX_INVENTORY_CAPACITY], int& inventoryCount,
                   bool objectives[NUM_OBJECTIVES], int& emergencyTripsLeft,
                   char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR - 1],
                   char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                   int entityCounts[NUM_FLOORS][ROOMS_PER_FLOOR]) {

    const int EMERGENCY_TRIPS_FROM_CARD = 3;
    const char* SAVE_FILE    = "out/savegame.txt";
    const char* SUMMARY_FILE = "out/game_summary.txt";

    // The player chooses the starting room
    bool startOk = false;
    while (startOk == false) {
        int startF = 0;
        int startR = 0;
        cout << "Choose your starting room (floor 1-" << NUM_FLOORS
                  << ", room 1-" << ROOMS_PER_FLOOR << "): ";
        if (!(cin >> startF >> startR)) {
            if (cin.eof()) {
                return;
            }
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Please enter two numbers.\n";
            continue;
        }
        if (startF >= 1 && startF <= NUM_FLOORS &&
            startR >= 1 && startR <= ROOMS_PER_FLOOR &&
            validateStartingRoom(startF, startR, horizontalConn)) {
            playerFloor = startF;
            playerRoom  = startR;
            startOk = true;
        } else {
            cout << "That is not a valid starting room.\n";
        }
    }

    cout << "Type 'help' for the list of commands.\n";
    renderRoomDetails(playerFloor, playerRoom, roomEntities,
                      entityCounts[playerFloor - 1][playerRoom - 1]);

    bool lift2Operational = false;
    bool quit = false;

    while (true) {
        // Lift 2 works normally only with the Fuse installed AND the Lift Pass
        lift2Operational = objectives[OBJ_FUSE_INSTALLED] && objectives[OBJ_LIFT_PASS];
        objectives[OBJ_LIFT2_ACTIVE] = lift2Operational;

        if (checkEndConditions(energy, time, objectives)) {
            break;
        }

        renderDashboard(playerFloor, playerRoom, energy, time, score,
                        inventory, inventoryCount, emergencyTripsLeft);
        cout << "> ";

        char cmd[MAX_NAME_LEN];
        if (!(cin >> setw(MAX_NAME_LEN) >> cmd)) {
            quit = true; // input closed
            break;
        }

        int roomIndexF = playerFloor - 1;
        int roomIndexR = playerRoom - 1;

        if (stringEquals(cmd, "move") || stringEquals(cmd, "go")) {
            int targetFloor = 0;
            int targetRoom = 0;
            if (!(cin >> targetFloor >> targetRoom)) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Usage: move <floor> <room>\n";
                continue;
            }
            bool moved = handleMoveCommand(targetFloor, targetRoom,
                                           playerFloor, playerRoom,
                                           energy, time,
                                           horizontalConn,
                                           inventory, inventoryCount,
                                           emergencyTripsLeft, lift2Operational);
            if (moved) {
                renderRoomDetails(playerFloor, playerRoom, roomEntities,
                                  entityCounts[playerFloor - 1][playerRoom - 1]);
            }

        } else if (stringEquals(cmd, "inspect")) {
            handleInspectCommand(playerFloor, playerRoom, energy, time,
                                 roomEntities, entityCounts[roomIndexF][roomIndexR],
                                 objectives, score);

        } else if (stringEquals(cmd, "collect")) {
            char symbol = ' ';
            if (!(cin >> symbol)) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Usage: collect <item symbol>\n";
                continue;
            }
            bool collected = handleCollectCommand(symbol, playerFloor, playerRoom,
                                                  energy, time, score,
                                                  roomEntities, entityCounts[roomIndexF][roomIndexR],
                                                  inventory, inventoryCount, objectives);
            if (collected && symbol == ENT_EMERGENCY_CARD) {
                emergencyTripsLeft = EMERGENCY_TRIPS_FROM_CARD;
                cout << "The card allows " << EMERGENCY_TRIPS_FROM_CARD
                          << " emergency trips on Lift 2.\n";
            }

        } else if (stringEquals(cmd, "use")) {
            char symbol = ' ';
            if (!(cin >> symbol)) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Usage: use <item symbol>\n";
                continue;
            }
            handleUseCommand(symbol, playerFloor, playerRoom, energy, time, score,
                             inventory, inventoryCount, objectives);

        } else if (stringEquals(cmd, "unlock")) {
            char code[MAX_NAME_LEN];
            if (!(cin >> setw(MAX_NAME_LEN) >> code)) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Usage: unlock <code>\n";
                continue;
            }
            bool opened = handleUnlockCabinetCommand(code, playerFloor, playerRoom,
                                                     objectives, score,
                                                     roomEntities,
                                                     entityCounts[roomIndexF][roomIndexR]);
            if (opened) {
                // Puzzle interaction costs Energy and Time only when it succeeds
                applyResourceDelta(energy, time, COST_ITEM_USE_ENERGY, COST_ITEM_USE_TIME);
            }

        } else if (stringEquals(cmd, "map")) {
            renderAsciiBuilding(playerFloor, playerRoom);

        } else if (stringEquals(cmd, "objectives")) {
            renderObjectivesChecklist(objectives);

        } else if (stringEquals(cmd, "save")) {
            if (saveGameStateToFile(SAVE_FILE, playerFloor, playerRoom,
                                    energy, time, score,
                                    inventory, inventoryCount,
                                    objectives, emergencyTripsLeft,
                                    roomEntities, entityCounts)) {
                cout << "Game saved.\n";
            } else {
                cout << "Could not save the game.\n";
            }

        } else if (stringEquals(cmd, "load")) {
            if (loadGameStateFromFile(SAVE_FILE, playerFloor, playerRoom,
                                      energy, time, score,
                                      inventory, inventoryCount,
                                      objectives, emergencyTripsLeft,
                                      roomEntities, entityCounts)) {
                cout << "Game loaded.\n";
                renderRoomDetails(playerFloor, playerRoom, roomEntities,
                                  entityCounts[playerFloor - 1][playerRoom - 1]);
            } else {
                cout << "Could not load a saved game.\n";
            }

        } else if (stringEquals(cmd, "help")) {
            cout << "Commands:\n"
                      << "  move <floor> <room>   move to a connected room\n"
                      << "  inspect               search the room (reveals clue digits)\n"
                      << "  collect <symbol>      pick up an item, e.g. collect <\n"
                      << "  use <symbol>          use an item, e.g. use *\n"
                      << "  unlock <code>         enter the cabinet code (in F5-R1)\n"
                      << "  map                   show the building\n"
                      << "  objectives            show the objectives checklist\n"
                      << "  save / load           save or load your game\n"
                      << "  quit                  end the game\n";

        } else if (stringEquals(cmd, "quit")) {
            quit = true;
            break;

        } else {
            cout << "Unknown command. Type 'help' for the list of commands.\n";
        }
    }

    // Final result
    bool victory = objectives[OBJ_MISSION_WON];
    if (victory == false) {
        objectives[OBJ_MISSION_FAILED] = true;
    }

    cout << "\n===========================================\n";
    if (victory) {
        cout << " MISSION COMPLETE - YOU CAUGHT THE LAST TRAIN\n";
    } else if (quit) {
        cout << " MISSION ABANDONED\n";
    } else {
        cout << " MISSION FAILED - YOU RAN OUT OF ENERGY OR TIME\n";
    }
    cout << " Final score: " << score << "\n";
    cout << "===========================================\n";

    writeGameSummaryReport(SUMMARY_FILE, score, energy, time, objectives, victory);
}
