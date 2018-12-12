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

// Magazine Servo Definitions
#define servoDataPin 10         // Using the outer 3-pin connector on the motorshield
// Position Stepper Definitions
#define stepperDataPin 2        // Equivalent to M3 & M4 on the motorshield
#define stepperSteps 200        // 360° total / 1,8° per step = 200 steps per revolution
#define stepperRPM 200          // Desired # of rotations per minute
#define stepperSPC 230          // Measured steps per column TODO!!!!
// Game Structure Definitions
#define columns 7               // The game's column number (absolute, > 0)
#define rows 6                  // The game's row number (absolute, > 0)
#define homeColumn 1            // The column at which positionStepper is positioned at the beginning
#define winningStreak 4         // The number of directly neighboring tiles needed to win
// PS/2 Mouse Definitions
#define mouseDataPin 1          // The mouse's PS/2 conforming data pin on Arduino (no PWM needed)
#define mouseClockPin 2         // The mouse's PS/2 conforming clock pin on Arduino (no PWM needed)
// LED Definitions
#define led1Pin 3               // The selection indicator LED pin for column 1
#define led2Pin 4               // ... 2
#define led3Pin 5               // ... 3
#define led4Pin 6               // ... 4
#define led5Pin 7               // ... 5
#define led6Pin 8               // ... 6
#define led7Pin 9               // ... 7
// Convenience Definitions
#define maxY columns - 1        // The biggest Y coordinate possible
#define maxX rows - 1           // The biggest X coordinate possible
#define homeX homeColumn - 1    // The home position's x-coordinate
#define U 0                     // An unowned/empty field representation
#define P 1                     // A player-owned field representation
#define C 2                     // A computer-owned field representation
#define nil 255                 // A definition for empty values in the game's logic, because C++ does NOT have a dedicated NULL/NIL; Note that it must be grater than the game logic's max number value of 7 and of byte type


// MARK: - Global properties

// Motor Objects
Adafruit_MotorShield motorshield = Adafruit_MotorShield();                                          // The motorshield object, initialized at the beginning
Adafruit_StepperMotor *positionStepper = motorshield.getStepper(stepperSteps, stepperDataPin);      // The stepper motor reference, retreived from the motorshield by defining the step # and data pin
Servo magazineServo = Servo();                                                                      // The servo object, initialized at the beginning
// Mouse Object
PS2Mouse mouse = PS2Mouse(mouseClockPin, mouseDataPin);                                             // The PS/2 mouse object, initialized at the beginning
// Game Logic Variables
int currentX = homeX;                                                    // The steppers current position measured as columnIndex
byte gameMap[6][7] = {                                                                              // The game's current state in memory
    {U, U, U, U, U, U, U},                                                                          // To retrieve a tile's status, call gameMap[y][x], where y ∈ [0;rows) and x ∈ [0;columns)
    {U, U, U, U, U, U, U},
    {U, U, U, U, U, U, U},
    {U, U, U, U, U, U, U},
    {U, U, U, U, U, U, U},
    {U, U, U, U, U, U, U},
};
byte ledPinMap[7] = {                                                                               // A convenince array for quickly accessing the right LED pin for a given column
    led1Pin, led2Pin, led3Pin, led4Pin, led5Pin, led6Pin, led7Pin                                   // To retrieve a column's LED pin, call ledPinMap[x], where x ∈ [0;columns)
};


// MARK: - Controller Lifecycle

/// A synchronous main function called once after the controller has booted up.
/// Parameters: *None*
/// Returns: *Nothing*
void setup() {
    // Establish Serial connection for debugging
	Serial.begin(57600);
    Serial.println("Tic-Tac-Toe!");
    // Initialize motorshield connection with its default frequency (1.6KHz PWM)
    motorshield.begin();
    // Configure magazineServo and set to center position
    magazineServo.attach(servoDataPin);
    magazineServo.write(70);
    // Configure positionStepper speed
    positionStepper->setSpeed(stepperRPM);
    // Configure LED pin modes
    configureLEDs();
    // Instantiate randomizer using noisy analog read on A4
    randomSeed(analogRead(A4));
    // Release stepper coils so that the user can position it a 0
    positionStepper->release();
    // Wait for user input before beginning
    Serial.println("USER ACTION NEEDED: Adjust position stepper to home directory and confirm by sending a character!");
    while (!Serial.available()) { /* Wait for serial event */ }
    // Empty Serial Buffer
    Serial.read();
    Serial.println("Starting game routines");
}

/// A synchronous main function called repeatedly after void setup().
/// Parameters: *None*
/// Returns: *Nothing*
void loop() {
    playPlayerMove();
    printGameMap();
    if (didWin(P)) { Serial.println("Player has won!"); delay(10000); }
    playCOMMove();
    printGameMap();
    if (didWin(C)) { Serial.println("COM has won!"); delay(10000); }
}


// MARK: - Gameplay Methods

/// A synchronous function that waits for the player's input and plays his move.
/// Parameters: *None*
/// Returns: *Nothing*
void playPlayerMove() {
    // Await the player's input
    Serial.println("USER ACTION NEEDED: Select your column in [1;columns]");
    while (!Serial.available()) { /* Wait for serial event */ }
    byte selectedX = Serial.parseInt() - 1;
    playMove(P, selectedX);
}

/// A synchronous function that plays a COM move based on certain algorithms and on random chance.
/// Parameters: *None*
/// Returns: *Nothing*
void playCOMMove() {
    Serial.println("COM: Playing my move");
    playMove(C, randomX());
}

/// A synchronous function that plays a specified move for a specified entity and writes changes to gameMap.
/// Parameters: - byte EID: The entity's ID, ∈ {C;P}
///             - byte x: The x-coordinate for the desired move, ∈ [0;columns)
/// Returns: *Nothing*
void playMove(byte EID, byte x) {
    // Turn on the LED
    digitalWrite(ledPinMap[x], HIGH);
    // Read from bottom to top
    for (byte y = maxY; y >= 0; y--) {
        // Wait for the first unowned row
        if (gameMap[y][x] == U) {
            // Save new ownership to gameMap and break for-loop
            gameMap[y][x] = EID;
            break;
        }
    }
    // Actually play the move
    moveToX(x);
    dispenseAndReloadMagazine();
    // Turn off the LED
    digitalWrite(ledPinMap[x], LOW);
}


// MARK: - Mechanical / Physical Methods

/// Sets the pin mode for all the LEDs in bulk
/// Parameters: - byte x: The x-coordinate for the desired move, ∈ [0;columns)
/// Returns: *Nothing*
void configureLEDs() {
    for (byte x = 0; x <= maxX; x++) {
        // Configure LED pins
        pinMode(ledPinMap[x], OUTPUT);
    }
}

/// A synchronous function that moves the physical cursor to the desired x-coordinate using positionStepper.
/// Parameters: - byte x: The x-coordinate for the desired move, ∈ [0;columns)
/// Returns: *Nothing*
void moveToX(byte x) {
    // Calculate the amount of steps that positionStepper has to perform; Note that this value might be of negative sign if the stepper has to move to the left!
    long neededSteps = stepperSPC * (x - currentX);
    // Interpret direction and move positionStepper based on neededSteps' sign
    if (neededSteps > 0) {
        // positionStepper has to move to the right
        positionStepper->step(neededSteps, FORWARD, DOUBLE);
    } else if (neededSteps < 0) {
        // positionStepper has to move to the left
        positionStepper->step(-neededSteps, BACKWARD, DOUBLE);
    }
    // Save the new x-coordinate
    currentX = x;
}

/// A synchronous function that dispenses a chip from the magazine and reloads it.
/// Parameters: *None*
/// Returns: *Nothing*
void dispenseAndReloadMagazine() {
    // Dispense chip
    magazineServo.write(170);
    delay(1000);
    // Reload magazine
    magazineServo.write(40);
    delay(1000);
    // Reset to center position
    magazineServo.write(70);
}


// MARK: - Gameplay Helper Methods

/// A synchronous function that generates a random, but valid move's x-Coordinate.
/// Parameters: *None*
/// Returns: A byte with the randomly selected x-Coordinate
byte randomX() {
    // Variable storing a random column
    byte randomX;
    // Populate with a random x-coordinate and repeat the process as long as no free column is found
    do { randomX = random(0, (maxX + 1)); } while (gameMap[0][randomX] != U);
    // Return the found value
    return randomX;
}

/// A synchronous function that checks whether the player has won by scanning gameMap for all winning patterns according to the game's rules
/// Parameters: - byte EID: The entity ID for which a win shall be checked (either C or P)
/// Returns: A boolean value specifying whether the player has won
bool didWin(byte EID) {
    Serial.println("Checking if this was a winning move");
    // Counter for directly neighboring tiles
    byte currentStreak;
    // Check winning rows, from bottom to top
    Serial.print("Scanning rows, ");
    for (byte y = maxY; y < 255; y--) {
        // Reset streak for each row
        currentStreak = 0;
        // Read from left to right
        for (byte x = 0; x <= maxX; x++) {
            // Player owns the field -> Increment currentStreak by 1
            if (gameMap[y][x] == EID) { currentStreak++; }
            // Player doesn't own the field -> Reset currentStreak to 0
            else { currentStreak = 0; }
            // Check currentStreak status and return true if won
            if (currentStreak == winningStreak) { return true; }
            // Stop checking this row if no win is possible with the remaining columns
            if (x + (winningStreak - currentStreak) > maxX) { break; }
        }
    }
    // Check winning colums, from left to right
    Serial.print("columns and ");
    for (byte x = 0; x <= maxX; x++) {
        // Reset streak for each column
        currentStreak = 0;
        // Read from bottom to top
        for (byte y = maxY; y < 255; y--) {
            // Player owns the field -> Increment currentStreak by 1
            if (gameMap[y][x] == EID) { currentStreak++; }
            // Player doesn't own the field -> Reset currentStreak to 0
            else { currentStreak = 0; }
            // Check currentStreak status and return true if won
            if (currentStreak == winningStreak) { return true; }
            // Stop checking this column if no win is possible with the remaining columns
            if (y - (winningStreak - currentStreak) > maxY) { break; }
        }
    }
    // Check winning bottom left to top right diagonals
    Serial.println("diagonals");
    // Check wins from possible origins for x = 0
    for (byte y = maxY; y >= (winningStreak - 1); y--) {
        // Reset streak for each diagonal
        currentStreak = 0;
        // Read from bottom left to top right
        for (byte diagonalIndex = 0; diagonalIndex <= y; diagonalIndex++) {
            // Player owns the field -> Increment currentStreak by 1
            if (gameMap[y - diagonalIndex][0 + diagonalIndex] == EID) { currentStreak++; }
            // Player doesn't own the field -> Reset currentStreak to 0
            else { currentStreak = 0; }
            // Check currentStreak status and return true if won
            if (currentStreak == winningStreak) { return true; }
        }
    }
    // Check wins from possible origins for x = maxX
    for (byte y = 0; y < (winningStreak - 1); y++) {
        // Reset streak for each diagonal
        currentStreak = 0;
        // Read from top right to bottom left
        for (byte diagonalIndex = 0; diagonalIndex <= (maxY - y); diagonalIndex++) {
            // Player owns the field -> Increment currentStreak by 1
            if (gameMap[y + diagonalIndex][maxX - diagonalIndex] == EID) { currentStreak++; }
            // Player doesn't own the field -> Reset currentStreak to 0
            else { currentStreak = 0; }
            // Check currentStreak status and return true if won
            if (currentStreak == winningStreak) { return true; }
        }
    }
    // Check winning top left to bottom right diagonals
    // Check wins from possible origins for x = 0
    for (byte y = 0; y < (winningStreak - 1); y++) {
        // Reset streak for each diagonal
        currentStreak = 0;
        // Read from top left to bottom right
        for (byte diagonalIndex = 0; diagonalIndex <= (maxY - y); diagonalIndex++) {
            // Player owns the field -> Increment currentStreak by 1
            if (gameMap[y + diagonalIndex][0 + diagonalIndex] == EID) { currentStreak++; }
            // Player doesn't own the field -> Reset currentStreak to 0
            else { currentStreak = 0; }
            // Check currentStreak status and return true if won
            if (currentStreak == winningStreak) { return true; }
        }
    }
    // Check wins from possible origins for x = maxX
    for (byte y = maxY; y >= (winningStreak - 1); y--) {
        // Reset streak for each diagonal
        currentStreak = 0;
        // Read from bottom right to top left
        for (byte diagonalIndex = 0; diagonalIndex <= y; diagonalIndex++) {
            // Player owns the field -> Increment currentStreak by 1
            if (gameMap[y - diagonalIndex][maxX - diagonalIndex] == EID) { currentStreak++; }
            // Player doesn't own the field -> Reset currentStreak to 0
            else { currentStreak = 0; }
            // Check currentStreak status and return true if won
            if (currentStreak == winningStreak) { return true; }
        }
    }
    // No winning line detected -> Return false!
    Serial.println("No winning line");
    return false;
}


// MARK: - Debugging Methods

/// A synchronous function that prints gameMap to the Serial Monitor, useful for debugging.
/// Parameters: *None*
/// Returns: *Nothing*
void printGameMap() {
    for (byte row = 0; row <= 5; row++) {
        for (byte column = 0; column <= 6; column++) {
            Serial.print(gameMap[row][column]); Serial.print(" ");
        }
        Serial.println();
    }
}

