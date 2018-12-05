// TIC-TAC-TOE
// A project by Pauline Hocher, Jonas Herrmann, Marcel Karas, Simeon Schwarzkopf & Alexander Zank
// Copyright (C) 2018 Alexander Zank
// Available under the MIT License, see README.md
// Last update on 05.12.2018

// MARK: - Dependencies

#include <PS2Mouse.h>
#include <Adafruit_MotorShield.h>
#include <Servo.h>

// MARK: - Global definitions

#define servoDataPin 10     // Using the outer 3-pin connector on the motorshield
#define stepperDataPin 1    // Equivalent to M1 & M2 on the motorshield
#define stepperSteps 200    // 360° total / 1,8° per step = 200 steps per revolution
#define stepperRPM 20       // Desired # of rotations per minute
#define mouseDataPin 1      // The mouse's PS/2 conforming data pin on Arduino (no PWM needed)
#define mouseClockPin 2     // The mouse's PS/2 conforming clock pin on Arduino (no PWM needed)


// MARK: - Global properties

Adafruit_MotorShield motorshield = Adafruit_MotorShield();                                          // The motorshield object, initialized at the beginning
Adafruit_StepperMotor *positionStepper = motorshield.getStepper(stepperSteps, stepperDataPin);      // The stepper motor reference, retreived from the motorshield by defining the step # and data pin
Servo magazineServo = Servo();   
PS2Mouse mouse = PS2Mouse()                                                                   // The servo object, initialized at the beginning


// MARK: - Main lifecycle

void setup() {
    // Establish Serial connection for debugging
	Serial.begin(9600);
    Serial.println("Tic-Tac-Toe!");
    // Configure magazineServo
    magazineServo.attach(servoDataPin);
    // Configure positionStepper speed
    positionStepper->setSpeed(stepperRPM);
    // Initialize mouse
    mouse.initialize();
}

void loop() {
    // DEBUG
    MouseData mouseData = mouse.readData();
    Serial.println(mouseData.position.x);
    Serial.println(mouseData.position.y)
    delay(20);
}

void serialEvent() {
    // Read incoming numbers byte-by-byte and save to property in local scope
    int serialInput = Serial.read();
    // DEBUG
    magazineServo.write(serialInput);
    positionStepper->step(serialInput, FORWARD, DOUBLE);
}
