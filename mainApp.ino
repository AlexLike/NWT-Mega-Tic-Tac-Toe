// TIC-TAC-TOE
// A project by Pauline Hocher, Jonas Herrmann, Marcel Karas, Simeon Schwarzkopf & Alexander Zank
// Copyright (C) 2018 Alexander Zank
// Available under the MIT License, see README.md
// Last update on 04.12.2018

// MARK: - Dependencies

#include <Adafruit_MotorShield.h>
#include <Servo.h>

// MARK: - Global definitions

#define servoDataPin 10     // Using the outer 3-pin connector on the motorshield
#define stepperDataPin 1    // Equivalent to M1 & M2 on the motorshield
#define stepperSteps 200    // 360° total / 1,8° per step = 200 steps per revolution
#define stepperRPM 20       // Desired # of rotations per minute


// MARK: - Global properties

Adafruit_MotorShield motorshield = Adafruit_MotorShield();                                          // The motorshield object, initialized at the beginning
Adafruit_StepperMotor *positionStepper = motorshield.getStepper(stepperSteps, stepperDataPin);      // The stepper motor reference, retreived from the motorshield by defining the step # and data pin
Servo magazineServo = Servo();                                                                      // The servo object, initialized at the beginning


// MARK: - Main lifecycle

void setup() {
    // Establish Serial connection for debugging
	Serial.begin(9600);
    Serial.println("Tic-Tac-Toe!");
    // Configure magazineServo
    magazineServo.attach(servoDataPin);
    // Configure positionStepper speed
    positionStepper->setSpeed(stepperRPM);
}

void loop() {
    // TODO
}

void serialEvent() {
    // Read incoming numbers byte-by-byte and save to property in local scope
    int serialInput = Serial.read();
    // DEBUG
    magazineServo.write(serialInput);
    positionStepper->step(serialInput, FORWARD, DOUBLE);
}
