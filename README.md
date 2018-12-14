# Tic-Tac-Toe
#### A project by Pauline Hocher, Jonas Herrmann, Marcel Karas, Simeon Schwarzkopf and Alexander Zank realized at Hohenstaufen-Gymnasium Bad Wimpfen during December 2018.

## Introduction
The goal of project Mega-Tic-Tac-Toe is the implementation of a mechanical player for a bigger sized "Connect 4" board. Programatically this is achieved by using an Arduino Genuino Uno and the code contained in this repository.

## Construction outline
A stepper motor-powered hinge allows the tile magazine to be moved around freely on top of the "Connect-4" board while the magazine's opening and reloading mechanism is realized by using a small servo. An LED strip on the bottom has been added for aesthetical reasons.

![Construction Outline](https://raw.githubusercontent.com/AlexLike/NWT-Mega-Tic-Tac-Toe/Documentation-Assets/Construction%20Overview.jpeg?token=AdDgy6LOyk1BvnHT4aixvDwxQrZ3rKcsks5cHRMuwA%3D%3D)

## Wiring outline
Adafruit's Motorshield V2 is stacked on the Arduino Genuino Uno and connected with an array of 4 AA-batteries that will later power the two motors when connected correctly. Every LED receives an own data pin and a shared connection to the Arduino's GND. The Arduino-to-Computer serial connection is realized via an USB Type B to A cable that connects the two to each other thereby occupying pin 0 and 1 on the board.

## Coding principles
This project's code was written in native C++ with the Arduino's base and servo libraries as well as Adafruit's motorshield library V2 stacked on top. A key design decision was emphasizing the modularity and clarity of the code and keeping it as lightweight as possible so that the Arduino's dynamic storage can be used to its fullest potential.

The code is clearly structured in the following sections, allowing any contributer to more easily navigate the small project:
```C++
// MARK: - Dependencies
// MARK: - Global definitions
// MARK: - Global properties
// MARK: - Controller Lifecycle
// MARK: - Gameplay Methods
// MARK: - Mechanical / Physical Methods
// MARK: - Gameplay Helper Methods
// MARK: - Debugging Methods
```

One measure to save dynamic storage while preserving the ease of use in the adjustment and calibration process (e.g. trying different `stepperSPC`) was the use of the C++-Keyword `#define` for any constant value in the code, as a declaration using said keyword isn't saved to the dynamic storage at runtime but rather filled in at compiletime. The Arduino has way more static storage for code than dynamic storage for changing variables, so using the latter allows for a better use of dynamic storage, e.g. the win-detection that uses multiple variables for scanning through rows.

When declaring variables, data types are also used sparingly which means that for most single variables and array objects, the `byte` type was used. We only have to store integer values in [0;9], so the byte's range of [0;255] is more than sufficient and there's no need to use `int` in most cases, saving 1 byte per instantiated variable. This may at first sound silly, but looking only at `gameMap` one reaches a saving of (6 ⨉ 7) Objects ⨉ 1 Byte saved per object = 42 bytes.

A greatly sophisticated subroutine is the `didWin(byte EID)` method. It scans each row, column and diagonal for a possible winning line by using `gameMap`'s data in `for`-loops and the provided entitie's ID defined at the beginning. This can either be P for Player or C for COM. The following diagram explains the scans `didWin(byte EID)` performs while running:



## License

>Copyright (c) 2018 Alexander Zank
>
>Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
>
>The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
>
>THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
