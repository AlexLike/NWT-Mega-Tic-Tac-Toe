// MEGA TIC-TAC-TOE (4-Wins)
// A project by Pauline Hocher, Jonas Herrmann, Marcel Karas, Simeon Schwarzkopf & Alexander Zank
// Copyright (C) 2018 Alexander Zank
// Available under the MIT License, see README.md
// Last update on 05.12.2018

// MARK: - Dependencies

#include <Adafruit_MotorShield.h>
#include <Servo.h>
#include <PS2Mouse.h>

// MARK: - Global definitions

#define servoDataPin 10     // Using the outer 3-pin connector on the motorshield
#define stepperDataPin 1    // Equivalent to M1 & M2 on the motorshield
#define stepperSteps 200    // 360° total / 1,8° per step = 200 steps per revolution
#define stepperRPM 20       // Desired # of rotations per minute
#define stepperSPC          // Measured steps per column
#define mouseDataPin 1      // The mouse's PS/2 conforming data pin on Arduino (no PWM needed)
#define mouseClockPin 2     // The mouse's PS/2 conforming clock pin on Arduino (no PWM needed)
#define led1Pin 3           // The selection indicator LED pin for column 1
#define led2Pin 4           // ... 2
#define led3Pin 5           // ... 3
#define led4Pin 6           // ... 4
#define led5Pin 7           // ... 5
#define led6Pin 8           // ... 6
#define led7Pin 9           // ... 7
#define winningStreak 4     // The number of directly neighboring tiles needed to win
#define O 0                 // An empty field representation
#define P 1                 // A player-owned field representation
#define C 2                 // A computer-owned field representation


// MARK: - Global properties

Adafruit_MotorShield motorshield = Adafruit_MotorShield();                                          // The motorshield object, initialized at the beginning
Adafruit_StepperMotor *positionStepper = motorshield.getStepper(stepperSteps, stepperDataPin);      // The stepper motor reference, retreived from the motorshield by defining the step # and data pin
Servo magazineServo = Servo();                                                                      // The PS/2 mouse object, initialized at the beginning
PS2Mouse mouse = PS2Mouse(mouseClockPin, mouseDataPin);                                             // The servo object, initialized at the beginning

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
    // Configure magazineServo and set to center position
    magazineServo.attach(servoDataPin);
    magazineServo.write(90);
    // Configure positionStepper speed
    positionStepper->setSpeed(stepperRPM);
    // Initialize mouse
    mouse.initialize();
    // Instantiate randomizer using noisy analog read on A4
    randomSeed(analogRead(A4));
}

void loop() {
    // DEBUG
    MouseData mouseData = mouse.readData();
    Serial.println(mouseData.position.x);
    Serial.println(mouseData.position.y);
    delay(20);
}


// MARK: - Custom methods

// Delegate functions
void serialEvent() {
    // Read incoming numbers byte-by-byte and save to property in local scope
    int serialInput = Serial.read();
    // DEBUG
    magazineServo.write(serialInput);
    positionStepper->step(serialInput, FORWARD, DOUBLE);
}

// Engine functions
void dispenseAndReloadMagazine() {
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
    // TODO: Use positionStepper ref to navigate to the desired column
}

// Gameplay logic functions

void playComMove() {
    
}

/// A function that plays a move for the player
/// Parameters: - byte column: The column to be played on
/// Returns: *Nothing*
void playMove(byte column) {
    positionAtColumn(column);
    dispenseAndReloadMagazine();
    if (didPlayerWin()) {
        // TODO: Flash lights or do something fancy! ;D
    }
}

/// A function that checks whether the player has won by scanning gameMap for all winning patterns according to the game's rules
/// Parameters: *None*
/// Returns: A boolean value specifying whether the player has won
bool didPlayerWin() {
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
            if (gameMap[row][column] == P) { currentStreak++; }
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
            if (gameMap[row][column] == P) { currentStreak++; }
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
            if (gameMap[row - diagonalIndex][0 + diagonalIndex] == P) { currentStreak++; }
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
            if (gameMap[row + diagonalIndex][6 - diagonalIndex] == P) { currentStreak++; }
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
            if (gameMap[row + diagonalIndex][0 + diagonalIndex] == P) { currentStreak++; }
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
            if (gameMap[row - diagonalIndex][6 - diagonalIndex] == P) { currentStreak++; }
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
