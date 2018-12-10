// MEGA TIC-TAC-TOE (4-Wins)
// A project by Pauline Hocher, Jonas Herrmann, Marcel Karas, Simeon Schwarzkopf & Alexander Zank
// Copyright (C) 2018 Alexander Zank
// Available under the MIT License, see README.md
// Last update on 05.12.2018

// MARK: - Dependencies

#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include <Servo.h>
#include <PS2Mouse.h>

// MARK: - Global definitions

#define servoDataPin 10         // Using the outer 3-pin connector on the motorshield
#define stepperDataPin 1        // Equivalent to M1 & M2 on the motorshield
#define stepperSteps 200        // 360° total / 1,8° per step = 200 steps per revolution
#define stepperRPM 200          // Desired # of rotations per minute
#define stepperSPC 1            // Measured steps per column TODO!!!!
#define homeColumnPosition 0    // The column at which positionStepper is positioned at the beginning
#define mouseDataPin 1          // The mouse's PS/2 conforming data pin on Arduino (no PWM needed)
#define mouseClockPin 2         // The mouse's PS/2 conforming clock pin on Arduino (no PWM needed)
#define led1Pin 3               // The selection indicator LED pin for column 1
#define led2Pin 4               // ... 2
#define led3Pin 5               // ... 3
#define led4Pin 6               // ... 4
#define led5Pin 7               // ... 5
#define led6Pin 8               // ... 6
#define led7Pin 9               // ... 7
#define winningStreak 4         // The number of directly neighboring tiles needed to win
#define O 0                     // An empty field representation
#define P 1                     // A player-owned field representation
#define C 2                     // A computer-owned field representation
#define nil 255                 // A definition for empty values in the game's logic, because C++ does NOT have a dedicated NULL/NIL; Note that it must be grater than the game logic's max number value of 7 and of byte type


// MARK: - Global properties

Adafruit_MotorShield motorshield = Adafruit_MotorShield();                                          // The motorshield object, initialized at the beginning
Adafruit_StepperMotor *positionStepper = motorshield.getStepper(stepperSteps, stepperDataPin);      // The stepper motor reference, retreived from the motorshield by defining the step # and data pin
Servo magazineServo = Servo();                                                                      // The PS/2 mouse object, initialized at the beginning
PS2Mouse mouse = PS2Mouse(mouseClockPin, mouseDataPin);                                             // The servo object, initialized at the beginning
int currentColumnPostition = homeColumnPosition;

byte gameMap[6][7] = {
    {O, O, O, O, O, O, O},
    {O, O, O, O, O, O, O},
    {O, O, O, O, O, O, O},
    {O, O, O, O, O, O, O},
    {O, O, O, O, O, O, O},
    {O, O, O, O, O, O, O},
};


// MARK: - Main lifecycle

void setup() {
    // Establish Serial connection for debugging
	Serial.begin(9600);
    Serial.println("Tic-Tac-Toe!");
    // Establish Motorshield connection with its default frequency (1.6KHz PWM)
    motorshield.begin();
    // Configure magazineServo and set to center position
    magazineServo.attach(servoDataPin);
    magazineServo.write(90);
    // Configure positionStepper speed
    positionStepper->setSpeed(stepperRPM);
    // Instantiate randomizer using noisy analog read on A4
    randomSeed(analogRead(A4));
    // Release stepper coils so that the user can position it a 0
    positionStepper->release();
    // Wait for user input before beginning
    Serial.println("USER ACTION NEEDED: Adjust position stepper to home directory and confirm by sending a character!");
    while (!Serial.available()) { /* Wait for serial event */ }
    Serial.println("Starting game routines");
}

void loop() {
    // COM's turn
    playCOMMove();
    if didWin(C) { Serial.println("The COM has won!!") }
    // Player's turn
    waitForAndPlayPlayerMove();
    if didWin(P) { Serial.println("The player has won!") }
}


// MARK: - Custom methods

// Delegate functions

void serialEvent() {
    // Read incoming numbers byte-by-byte and save to property in local scope
    int serialInput = Serial.read();
    Serial.print("Received"); Serial.println(serialInput);
    // DEBUG
    magazineServo.write(serialInput);
    positionStepper->step(serialInput, FORWARD, DOUBLE);
}

// Engine functions

void dispenseAndReloadMagazine() {
    Serial.println("DEBUG: Dispensing and reloading the magazine");
    // Dispense chip
    magazineServo.write(180);
    delay(1000);
    // Reload magazine
    magazineServo.write(60);
    delay(1000);
    // Reset to center position
    magazineServo.write(90);
}

void positionAtColumn(byte column) {
    Serial.println("DEBUG: Positioning at column index "); Serial.print(column);
    // Compute step count from current position
    int stepsToMove = stepperSPC * (currentColumnPostition - column);
    // Check direction by inspecting stepsToMove's sign and perform movement in the right direction
    if (stepsToMove < 0) { positionStepper->step(-stepsToMove, BACKWARD, DOUBLE); }
    else { positionStepper->step(stepsToMove, FORWARD, DOUBLE); }
    // Save new position to currentColumnPostition
    currentColumnPostition = column;
}

// Gameplay logic functions

/// A synchronous function that plays a COM-generated move
/// Parameters: *None*
/// Returns: *Nothing*
void playCOMMove() {
    // 1. If COM can win, play the winning move
    if (byte ownWinningMove = winningColumnFor(C) && ownWinningMove != nil) { playMove(ownWinningMove); }
    // 2. If player can win, stop him by playing his winning move
    else if (byte opponentWinningMove = winningColumnFor(P) && opponentWinningMove != nil) { playMove(opponentWinningMove); }
    // 3. Play a random, but possible move
    else { playMove(randomMove()); }
}

// A synchronous function that waits for the user's input and then plays his move
/// Parameters: *None*
/// Returns: *Nothing*
void waitForAndPlayPlayerMove() {
    // Wait for the user's move input
    Serial.println("USER ACTION NEEDED: Select your column");
    while (!Serial.available()) { /* Wait for serial event */ }
    byte selectedColumn = Serial.read() - 1;
    playMove(selectedColumn);
}

/// A synchronous function that plays a move
/// Parameters: - byte column: The column to be played on
/// Returns: *Nothing*
void playMove(byte column) {
    // Return eary if the move isn't possible
    if (!canReachField(0, column)) { return; }
    // Play the move!
    positionAtColumn(column);
    dispenseAndReloadMagazine();
}

/// A synchronous function that checks whether the player has won by scanning gameMap for all winning patterns according to the game's rules
/// Parameters: - byte EID: The entity ID for which a win shall be checked (either C or P)
/// Returns: A boolean value specifying whether the player has won
bool didWin(byte EID) {
    Serial.println("Checking, if the player has won...");
    // Counter for directly neighboring tiles
    byte currentStreak;
    // Check winning rows, from bottom to top
    for (byte row = 5; row >= 0; row--) {
        // Reset streak for each row
        currentStreak = 0;
        // Read from left to right
        for (byte column = 0; column <= 6; column++) {
            // Player owns the field -> Increment currentStreak by 1
            if (gameMap[row][column] == EID) { currentStreak++; }
            // Player doesn't own the field -> Reset currentStreak to 0
            else { currentStreak = 0; }
            // Check currentStreak status and return true if won
            if (currentStreak == winningStreak) { return true; }
            // Stop checking this row if no win is possible with the remaining columns
            if (column + (winningStreak - currentStreak) <= 6) { break; }
        }
    }
    // Check winning colums, from left to right
    for (byte column = 0; column <= 6; column++) {
        // Reset streak for each column
        currentStreak = 0;
        // Read from bottom to top
        for (byte row = 5; row >= 0; row--) {
            // Player owns the field -> Increment currentStreak by 1
            if (gameMap[row][column] == EID) { currentStreak++; }
            // Player doesn't own the field -> Reset currentStreak to 0
            else { currentStreak = 0; }
            // Check currentStreak status and return true if won
            if (currentStreak == winningStreak) { return true; }
            // Stop checking this column if no win is possible with the remaining columns
            if (row + (winningStreak - currentStreak) <= 5) { break; }
        }
    }
    // Check winning bottom left to top right diagonals
    // Check wins from possible origins in the left column
    for (byte row = 5; row >= 3; row--) {
        // Reset streak for each diagonal
        currentStreak = 0;
        // Read from bottom left to top right
        for (byte diagonalIndex = 0; diagonalIndex <= row; diagonalIndex++) {
            // Player owns the field -> Increment currentStreak by 1
            if (gameMap[row - diagonalIndex][0 + diagonalIndex] == EID) { currentStreak++; }
            // Player doesn't own the field -> Reset currentStreak to 0
            else { currentStreak = 0; }
            // Check currentStreak status and return true if won
            if (currentStreak == winningStreak) { return true; }
            // Stop checking this diagonal if no win is possible with the remaining tiles
            if (diagonalIndex + (winningStreak - currentStreak) <= row) { break; }
        }
    }
    // Check wins from (remaining) possible origins on the right column
    for (byte row = 0; row <= 2; row++) {
        // Reset streak for each diagonal
        currentStreak = 0;
        // Read from top right to bottom left
        for (byte diagonalIndex = 0; diagonalIndex <= 5 - row; diagonalIndex++) {
            // Player owns the field -> Increment currentStreak by 1
            if (gameMap[row + diagonalIndex][6 - diagonalIndex] == EID) { currentStreak++; }
            // Player doesn't own the field -> Reset currentStreak to 0
            else { currentStreak = 0; }
            // Check currentStreak status and return true if won
            if (currentStreak == winningStreak) { return true; }
            // Stop checking this diagonal if no win is possible with the remaining tiles
            if (diagonalIndex + (winningStreak - currentStreak) <= 5 - row) { break; }
        }
    }
    // Check winning top left to bottom right diagonals
    // Check wins from possible origins in the left column
    for (byte row = 0; row <= 2; row++) {
        // Reset streak for each diagonal
        currentStreak = 0;
        // Read from top left to bottom right
        for (byte diagonalIndex = 0; diagonalIndex <= 5 - row; diagonalIndex++) {
            // Player owns the field -> Increment currentStreak by 1
            if (gameMap[row + diagonalIndex][0 + diagonalIndex] == EID) { currentStreak++; }
            // Player doesn't own the field -> Reset currentStreak to 0
            else { currentStreak = 0; }
            // Check currentStreak status and return true if won
            if (currentStreak == winningStreak) { return true; }
            // Stop checking this diagonal if no win is possible with the remaining tiles
            if (diagonalIndex + (winningStreak - currentStreak) <= 5 - row) { break; }
        }
    }
    // Check wins from (remaining) possible origins on the right column
    for (byte row = 5; row >= 3; row--) {
        // Reset streak for each diagonal
        currentStreak = 0;
        // Read from bottom right to top left
        for (byte diagonalIndex = 0; diagonalIndex <= row; diagonalIndex++) {
            // Player owns the field -> Increment currentStreak by 1
            if (gameMap[row - diagonalIndex][6 - diagonalIndex] == EID) { currentStreak++; }
            // Player doesn't own the field -> Reset currentStreak to 0
            else { currentStreak = 0; }
            // Check currentStreak status and return true if won
            if (currentStreak == winningStreak) { return true; }
            // Stop checking this diagonal if no win is possible with the remaining tiles
            if (diagonalIndex + (winningStreak - currentStreak) <= row) { break; }
        }
    }
    // No winning row detected -> Return false!
    return false;
}

/// A synchronous function that checks whether the provided field can be accessed currently in one move
/// Parameters: - byte row: The field's row index in [0; 5]
///             - byte column: The field's column index in [0; 6]
/// Returns: A boolean value specifying whether the field can be accessed
bool canReachField(byte row, byte column) {
    Serial.println("Checking field availability");
    // Read from bottom to top
    for (byte currentRow = 5; row >= 0; row--) {
        // One of the underlying fields is empty -> Tile would fall through, therefore return false and exit closure early
        if (currentRow < row && gameMap[currentRow][column] == O) { return false; }
        // The desired field or one of the above fields isn't empty -> Tile wouldn't reach desired row, therefore return false and exit closure early
        if (currentRow >= row && gameMap[currentRow][column] != O) { return false; }
    }
    // The whole column has been checked an no problems have been found -> Return true
    return true;
}

/// A synchronous function that computes a column-winning move if possible
/// Parameters: - byte EID: The game entity's ID for which possibilities shall be checked (P or C)
/// Returns: The column index for the winning move if possible, or nil if not
/// WARNING: Incomplete implementation due to time issues: This function only searches columns, not rows or diagonals!
byte winningColumnFor(byte EID) {
    Serial.println("Computing winning move");
    // Counter for neighboring tiles
    byte currentStreak;
    // Check winnable columns from left to right
    for (byte column = 0; column <= 6; column++) {
        // Reset currentStreak for each column
        currentStreak = 0;
        // Read from bottom to top
        for (byte row = 5; row >= 0; row--) {
            // Assess field value
            switch (gameMap[row][column]) {
                // Field is owned by the provided entity -> Increase currentStreak
                case 1: //EID
                    currentStreak++;
                    break;
                // Field is unowned -> Check currentStreak
                case O:
                    // The unowned field leads to victory and is accessible -> Return this winning column
                    if (currentStreak == winningStreak - 1 && canReachField(row, column)) { return column; }
                    // The unowned field doesn't lead to victory or is inaccessible -> Reset currentStreak
                    else { currentStreak = 0; }
                    break;
                // Field is owned by the provided entity's opponent -> Reset currentStreak
                default:
                    currentStreak = 0;
                    break;
            }
        }
    }
}

/// A synchronous function that generates a random, but valid move
/// Parameters: *None*
/// Returns: A byte with the selected column index
byte randomMove() {
    // Variable storing a random column
    byte randomMove;
    // Populate with a random column # and repeat the process as long as a free column is found
    do { randomMove = random(0, 7); } while (gameMap[0][randomMove] != O);
    // Return the found value
    return randomMove;
}

















// // A function that computes the best move for the COM-Player
// // Parameters: *None*
// // Returns: A byte value in [0;6] representing the columnIndex to play
// byte computeBestMoveColumnIndex() {
//     Serial.println("Computing best move for COM player...");
//     // Counter for directly neighboring tiles
//     byte currentStreak;
//     // Check winnable rows, from bottom to top
//     for (byte row = 5; row >= 0; row--) {
//         // Redefine empty tiles' empty coordinates for each row
//         byte emptyTileColumnBeforeCOMLine = nil;
//         byte emptyTileColumnAfterCOMLine = nil;
//         byte emptyTileColumnInCOMLine = nil;
//         // Reset streak for each row
//         currentStreak = 0;
//         // Read from left to right
//         for (byte column = 0; column <= 6; column++) {
//             // Get the curent tile's occupation
//             byte currentTileValue = gameMap[row][column];
//             // COM owns the field -> Increment currentStreak by 1
//             if (currentTileValue == C) { currentStreak++; }
//             // Player owns the field -> Reset currentStreak to 0
//             else if (gameMap[row][column] == P) { currentStreak = 0; }
//             // Field is unowned -> Check for streak
//             else {
//                 // First field in possibly the winning line -> Store column
//                 if (currentStreak == 0) {
//                     emptyTileColumnBeforeCOMLine = column;
//                 // Last field in the winning line -> Store coordinates
//                 } else if (currentStreak == winningStreak - 1) {
//                     emptyTileColumnAfterCOMLine = column;
//                 // Field in the middle of possibly the winning line -> Store coordinates
//                 } else if (emptyTileColumnInCOMLine == nil) {
//                     emptyTileColumnInCOMLine = column;
//                 // 2nd empty field in a row -> Reset streak
//                 } else {
//                     emptyTileColumnInCOMLine = 0;
//                     currentStreak = 0;
//                 }
//             }
//         }
//         // Check if a win is possible, than decide on move
//         // Line can't be filled out in a row -> Continue reading next row
//         if (emptyTileColumnInCOMLine == nil && emptyTileColumnBeforeCOMLine == nil && emptyTileColumnAfterCOMLine == nil) { continue; }
//         // Line can be filled out from its inside
//         if (emptyTileColumnInCOMLine != nil) { return emptyTileColumnInCOMLine; }
//         // Line can either be filled out from both ends
//         if (emptyTileColumnBeforeCOMLine != nil && emptyTileColumnAfterCOMLine != nil) {
//             // Choose left / right completion randomly
//             byte randomIndex = random(0;2);
//             if (randomIndex == 0) { return emptyTileColumnBeforeCOMLine; }
//             else { return emptyTileColumnAfterCOMLine; }
//         // Line can be filled out from the left
//         } else if (emptyTileColumnBeforeCOMLine != nil) { return emptyTileColumnBeforeCOMLine; }
//         // Line can be filled out from the right
//         else { return emptyTileColumnAfterCOMLine; }
//     }
//     // TODO
//     // Check winnable colums, from left to right
//     for (byte column = 0; column <= 6; column++) {
//         // Reset top most empty tile availability bool for each column
//         bool emptyTileOnTop = false;
//         // Reset streak for each column
//         currentStreak = 0;
//         // Read from bottom to top
//         for (byte row = 5; row >= 0; row--) {
//             // Get the curent tile's occupation
//             byte currentTileValue = gameMap[row][column];
//             // COM owns the field -> Increment currentStreak by 1 and reset empty field availability
//             if (currentTileValue == C) { 
//                 currentStreak++;
//                 emptyTileOnTop = false;
//             // Player owns the field -> Reset currentStreak to 0 and reset empty field availability
//             } else if (gameMap[row][column] == P) {
//                 currentStreak = 0;
//                 emptyTileOnTop = false;
//             // Field is unowned -> Save
//             } else { emptyTileOnTop = true; }
//         }
//         // Check if a win is possible, than decide on move
//         // Line can be filled from the top
//         if (currentStreak == winningStreak - 1 && emptyTileOnTop) { return column; }
//         // Line can't be filled in this column
//         else { continue; }
//     }
//     // Check winning bottom left to top right diagonals
//     // Check wins from possible origins in the left column
//     for (byte row = 5; row >= 3; row--) {
//         // Get the curent tile's occupation
//             byte currentTileValue = gameMap[row][column];
//             // COM owns the field -> Increment currentStreak by 1
//             if (currentTileValue == C) { currentStreak++; }
//             // Player owns the field -> Reset currentStreak to 0
//             else if (gameMap[row][column] == P) { currentStreak = 0; }
//             // Field is unowned -> Check for streak
//             else {
//                 // First field in possibly the winning line -> Store column
//                 if (currentStreak == 0) {
//                     emptyTileColumnBeforeCOMLine = column;
//                 // Last field in the winning line -> Store coordinates
//                 } else if (currentStreak == winningStreak - 1) {
//                     emptyTileColumnAfterCOMLine = column;
//                 // Field in the middle of possibly the winning line -> Store coordinates
//                 } else if (emptyTileColumnInCOMLine == nil) {
//                     emptyTileColumnInCOMLine = column;
//                 // 2nd empty field in a row -> Reset streak
//                 } else {
//                     emptyTileColumnInCOMLine = 0;
//                     currentStreak = 0;
//                 }
//             }
//     }
//     // Check wins from (remaining) possible origins on the right column
//     for (byte row = 0; row <= 2; row++) {
//         // Reset streak for each diagonal
//         currentStreak = 0;
//         // Read from top right to bottom left
//         for (byte diagonalIndex = 0; diagonalIndex <= 5 - row; diagonalIndex++) {
//             // Player owns the field -> Increment currentStreak by 1
//             if (gameMap[row + diagonalIndex][6 - diagonalIndex] == P) { currentStreak++; }
//             // Player doesn't own the field -> Reset currentStreak to 0
//             else { currentStreak = 0; }
//             // Check currentStreak status and return true if won
//             if (currentStreak == winningStreak) { return true; }
//             // Stop checking this diagonal if no win is possible with the remaining tiles
//             if (diagonalIndex + (winningStreak - currentStreak) <= 5 - row) { break; }
//         }
//     }
//     // Check winning top left to bottom right diagonals
//     // Check wins from possible origins in the left column
//     for (byte row = 0; row <= 2; row++) {
//         // Reset streak for each diagonal
//         currentStreak = 0;
//         // Read from top left to bottom right
//         for (byte diagonalIndex = 0; diagonalIndex <= 5 - row; diagonalIndex++) {
//             // Player owns the field -> Increment currentStreak by 1
//             if (gameMap[row + diagonalIndex][0 + diagonalIndex] == P) { currentStreak++; }
//             // Player doesn't own the field -> Reset currentStreak to 0
//             else { currentStreak = 0; }
//             // Check currentStreak status and return true if won
//             if (currentStreak == winningStreak) { return true; }
//             // Stop checking this diagonal if no win is possible with the remaining tiles
//             if (diagonalIndex + (winningStreak - currentStreak) <= 5 - row) { break; }
//         }
//     }
//     // Check wins from (remaining) possible origins on the right column
//     for (byte row = 5; row >= 3; row--) {
//         // Reset streak for each diagonal
//         currentStreak = 0;
//         // Read from bottom right to top left
//         for (byte diagonalIndex = 0; diagonalIndex <= row; diagonalIndex++) {
//             // Player owns the field -> Increment currentStreak by 1
//             if (gameMap[row - diagonalIndex][6 - diagonalIndex] == P) { currentStreak++; }
//             // Player doesn't own the field -> Reset currentStreak to 0
//             else { currentStreak = 0; }
//             // Check currentStreak status and return true if won
//             if (currentStreak == winningStreak) { return true; }
//             // Stop checking this diagonal if no win is possible with the remaining tiles
//             if (diagonalIndex + (winningStreak - currentStreak) <= row) { break; }
//         }
//     }
// }