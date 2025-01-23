/*
 Name: OBD2_Interface_v2.ino
Description: Designed specifically for connection to a Land Rover Freelander 2 (VIN: >382339)
Created: Sep 2024
Author: Kevin Guest AKA TheBionicBone

Disclaimer:
THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES, OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT, OR OTHERWISE, ARISING FROM, OUT OF, OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

THIS SOFTWARE AND HARDWARE ARE PROVIDED FOR EDUCATIONAL AND EXPERIMENTAL PURPOSES ONLY. IT IS THE USER'S
RESPONSIBILITY TO ENSURE COMPLIANCE WITH ALL APPLICABLE LAWS AND REGULATIONS REGARDING VEHICLE DATA ACCESS AND
MODIFICATION.

USE THIS DEVICE AND SOFTWARE AT YOUR OWN RISK. INTERACTING WITH VEHICLE SYSTEMS MAY RESULT IN UNINTENDED
CONSEQUENCES, INCLUDING BUT NOT LIMITED TO ERRORS, SYSTEM MALFUNCTIONS, OR DATA LOSS. THE AUTHORS ACCEPT NO
RESPONSIBILITY FOR DAMAGE TO VEHICLES, DATA, OR OTHER PROPERTY RESULTING FROM THE USE OF THIS DEVICE OR SOFTWARE.
*/

// Library: arduino-mcp2515 by autowp information, the follow license should be observed
/*
  This code uses the MCP2515 CANBUS Library:
  arduino-mcp2515 by autowp
  https://github.com/autowp/arduino-mcp2515
  The MIT License (MIT)
  Copyright (c) 2013 Seeed Technology Inc.
  Copyright (c) 2016 Dmitry

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/
#include <mcp2515.h>
const String LIBRARY_NAME = "arduino_mpc2515 (by autowp)";

// Library: TFT_ePSI by Bodmer information, the follow license should be observed
/*
  Software License Agreement (FreeBSD License)

  Copyright (c) 2023 Bodmer (https://github.com/Bodmer)

  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice, this
     list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation
     and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  The views and conclusions contained in the software and documentation are those
  of the authors and should not be interpreted as representing official policies,
  either expressed or implied, of the FreeBSD Project.
*/
/*  For the Display & Touch, I am using TFT_eSPI by Bodmer
    See Setup42_ILI9341_ESP32.h for more information
    External Setup Required:-
    * Ensure you are using the Library contained within this project
    * Key points are:
    *   User_Setup_Select.h is pointing to Setup42
    *   Setup42_ILI9341_ESP32.h from this project is being used because this has the correct pins and other requirements like HSPI
*/
#include <TFT_eSPI.h>
#include <FS.h>                                                       // TFT_eSPI requires this arduino-ESP32 core library
#include <SPIFFS.h>                                                   // TFT_eSPI requires this arduino-ESP32 core library
#include <vfs_api.h>                                                  // TFT_eSPI requires this arduino-ESP32 core library
#include <FSImpl.h>                                                   // TFT_eSPI requires this arduino-ESP32 core library
TFT_eSPI TFT_Rectangle_ILI9341 = TFT_eSPI();

//// include my Keyboard library
//#include "TFT_eSPI_Keyboard.h"
//TFT_eSPI_Keyboard Keyboard(TFT_Rectangle_ILI9341);  // Pass the TFT_eSPI instance to the keyboard library

// include the arduino-ESP32 core SPI library which is used for the MCP2515 controllers and the ILI9341 Display
#include <SPI.h>
// include the arduino-ESP32 core HardwareSerial library which is used for the SD Card Writer
#include <HardwareSerial.h>

// Connections to the ESP32-S3 Follow:
/*
  Power 5v Switched Supply
  GND = GND
  12v = VCC In
  5v Out = 5v
  
  ESP32-S3 & two MCP2515 Breakout Board (HW-184) - 8 MHz Crystal
  MCP2515_0 for 500kbps CAN Bus
  SCK = pin 12
  SI = pin 11
  SO = pin 13
  CS = pin 14
  GND = GND
  VCC = 5v

  MCP2515_1 for 125kbps CAN Bus
  SCK = pin 12
  SI = pin 11
  SO = pin 13
  CS = pin 10
  GND = GND
  VCC = 5v

  ESP32-S3 OpenLager (SD Card) Connections ("OpenLager" device should not be confused with the slower "OpenLogger") 
  Rx = Pin 9
  GND = GND
  VCC = 5v

  ESP32-S3 ILI9341 Display (with touch) Connections
  T_IRQ = Not Connected
  T_DO = 38
  T_DIN = 41
  T_CS = 37
  T_CLK = 40
  MISO = 38
  LED = 39 (2.5mA)
  SCK = 40
  MOSI = 41
  DC = 42
  RESET = 2
  CS = 1
  GND = GND
  VCC = 3.3v

  Stop Button
  GND = GND
  Botton = 21
*/


// Debugging Options (ESP32 Version)
// DEBUG 0 = Debugging Serial Messages are switched off
// DEBUG 1 = Debugging Serial Messages are switched on
// DEBUG 2 = Dedugging Only Special Serial Messages are switch on

#define DEBUG 0

#if DEBUG == 1
#define debugLoop(fmt, ...) Serial.printf("%s: " fmt "\r\n", __func__, ##__VA_ARGS__) // Serial Debugging On
#else
#define debugLoop(fmt, ...)                                           // Serial Debugging Off
#endif
#if DEBUG == 2
#define debugSpecial(fmt, ...) Serial.printf("%s: " fmt "\r\n", __func__, ##__VA_ARGS__) // Serial Debugging On
#else
#define debugSpecial(fmt, ...)                                        // Serial Debugging Off
#endif


// Program Send Message Options
/*
  **************************************************************
  *                         CAUTION                            *
  * Sending messages to a car can have disastrous consequences *
  *     Only send the correct messages for your car !!         *
  *                 YOU HAVE BEEN WARNED !!                    *
  **************************************************************
*/
// ALLOW_SENDING_DATA_TO_CAN_BUS 0 = Safe, even if a send message is attempted it will be ignored
// ALLOW_SENDING_DATA_TO_CAN_BUS 1 = Caution, send message requests will be sent to the vehicle regardsless of consequences
#define ALLOW_SENDING_DATA_TO_CAN_BUS 1


#if ALLOW_SENDING_DATA_TO_CAN_BUS == 1
#warning "SENDING ACTIVATED - CAUTION !!"
#endif

// Enums
enum          CAN_INTERFACES {
              CAN1 =    16,
              CAN2 =    32,
              CANBOTH = 64,
};
enum          MESSAGE_BOX_BUTTONS {
              BTN_OK =      1,
              BTN_IGNORE =  2,
              BTN_CANCEL =  4,
              BTN_STOP =    8, 
              BTN_CAN1 =    16,
              BTN_CAN2 =    32,
              BTN_CANBOTH = 64,
              BTN_YES =     128,
              BTN_NO =      256};
enum          BUTTON_TYPES { 
              MENU, 
              MESSAGE_BOX, 
              DISPLAY_BUTTON };
enum          OUTPUT_TYPES { 
              OUTPUT_ANALYSE_CAN_BUS_RESULTS, 
              OUTPUT_FORMAT_CANDRIVE, 
              OUTPUT_FORMAT_SAVVYCAN, 
              OUTPUT_SD_CARD_SAVVYCAN, 
              OUTPUT_WIFI,
              DISPLAY_POINT_IN_TIME,
              DISPLAY_TESTING_DATA};


// Constants & ESP32-S3 pin declarations
const auto    STANDARD_SERIAL_OUTPUT_BAUD = 2000000;                  // Must be at least 2,000,000 to keep up with Land Rover Freelander 2 
const auto    SD_PORT_HARDWARE_SERIAL_NUMBER = 1;                     // The OpenLager will be connected to Hardware Serial Port 1
const auto    SD_CARD_ESP32_S3_TX_PIN = 9;                            // The OpenLager Rx pin will be connected to this ESP32-S3 Tx pin
const auto    CAN_BUS_0_CS_PIN = 14;                                  // The CS pin of the MCP2515 (High Speed CAN Bus 500kbps) will be connected to this ESP32-S3 pin
const auto    CAN_BUS_1_CS_PIN = 10;                                  // The CS pin of the MCP2515 (Medium Speed CAN Bus 125kbps) will be connected to this ESP32-S3 pin
const auto    STOP_BUTTON_PIN = 21;                                   // Physical Button to stop outputs (touch causes delays and loss of data)
const auto    TFT_LANDROVERGREEN = 12832;                             // A Land Rover Green RBG colour
const auto    TFT_Rectangle_ILI9341_LEDPIN = 39;                      // The ILI9341 display LED PIN will be connected to this ESP32-S3 pin
const auto    CALIBRATION_FILE = "/TouchCalData1";                    // This is the file name used to store the touch display calibration data, must start with /
const auto    REPEAT_CALIBRATION = false;                             // Set to true to always calibrate / recalibrate the touch display
const auto    TOP_MENU_FONT = 2;                                      // Font used for selection menu that is at the top of the display
const auto    TOP_MENU_Y_OFFSET = 10;                                 // Used to position the top menu just under the Program Title
const auto    TOP_MENU_PROGRAM_NAME = "OBD2 Interface by Bionicbone"; // Will be displayed at the top of the display
const auto    TOP_MENU_PROGRAM_VERSION = "v2.0.0(RC)";                    // Will be displayed at the top of the display
const auto    MENU_BOLD_FONT = FreeSansBoldOblique9pt7b;              // Font used for selection menus headers
const auto    MENU_FONT = FreeSansOblique9pt7b;                       // Font used for selection menus buttons
const auto    MENU_Y_OFFSET = 68;                                     // Used to position the menu just under the Menu Header

// Structures
struct CANBusConfiguration {
  uint16_t Interface1_Speed = 1000;
  uint16_t Interface2_Speed = 1000;
};

CANBusConfiguration CANBusConf;
                                                                      
// Control variables
bool          headerFirstRun = false;                                 // Will set to true when an serial output initialises, trigger for header
bool          sdCardFirstRun = false;                                 // Will set to true when SD Card initialises, trigger for SavvyCAN header
bool          CANBusFirstRun = false;                                 // Will set to true when CAN Bus mode changes, trigger for cfps timer to start
uint32_t      totalCANReceiveTimeTimer = 0;                           // Times how long we have been receiving CAN Frames
uint16_t      numberOfCANFramesReceived[2] = { 0,0 };                 // Counts the number of CAN Frames received
uint8_t       outputFormat = false;                                   // Tracks the required output type
uint32_t      upTimer = micros();                                     // Tracks how long the program has been running, used in the outputs
uint8_t       interfaceNumber = 0;                                    // Could be BTN_CAN1, BTN_CAN2, BTN_CANBOTH and controls the outputs
uint16_t      displayPointInTime = false;                             // Some outputs allow display of Point in Time data with reduced CAN Frames

// Sets the new output requirements
void actionOutputChange(uint16_t arg) {
  debugLoop("arg = %d\n", arg);
  outputFormat = arg;
  switch (outputFormat) {
  case OUTPUT_ANALYSE_CAN_BUS_RESULTS:
    OutputAnalyseCANBusResults();
    break;

  case OUTPUT_FORMAT_CANDRIVE:
    StartReadingCanBus();
    break;

  case OUTPUT_FORMAT_SAVVYCAN:
    StartReadingCanBus();
    break;

  case OUTPUT_SD_CARD_SAVVYCAN:
    StartReadingCanBus();
    break;

  case DISPLAY_POINT_IN_TIME:
    StartReadingCanBus();
    break;

  case DISPLAY_TESTING_DATA:
    StartReadingCanBus();
    break;

  default:
    break;
  }
}


// Allows user to configure the CAN BUS Settings
void changeCANSettings(uint16_t arg) {
  // TODO write the script
  // TODO I would like an "Auto Detect CAN Bus Speed" function, but first be 100% which CAN library I will be using
  debugLoop("arg = %d\n", arg);
}
                                                                      
     
// Create menus (each menu must have no more than 5 options to choose)
// The order of the menus in the code must consider the order of definitions
// Thus the structure is at the top and the highest menu hierarchy is at the bottom

// Menu Structure
enum { H, M, A };                                                     // H = Header, M = New Menu, A = Action Menu Option 
struct Menu {
  const char* text;
  uint16_t          action;
  Menu* menu;
  void (*func) (uint16_t);
  uint16_t          arg;
};

// Create the menus
// H = Header, M = New Menu, A = Action Menu Option 
Menu CANSettings[]{
  { "CAN Bus Settings", H },
  { "CAN Bus 0 Speed", A, 0, changeCANSettings, 0 },
  { },                                                                // Menu Terminator
};

Menu menuDisplay[]{
  { "Select Required Output", H },
  { "Point in Time", A, 0, actionOutputChange, DISPLAY_POINT_IN_TIME },
  { "Testing Data", A, 0, actionOutputChange, DISPLAY_TESTING_DATA },
  { },                                                                // Menu Terminator
};

Menu menuOutput[]{
  { "Select Required Output", H },
  { "Serial CanDrive", A, 0, actionOutputChange, OUTPUT_FORMAT_CANDRIVE },
  { "Serial SavvyCAN", A, 0, actionOutputChange, OUTPUT_FORMAT_SAVVYCAN },
  { "Save SavvyCAN", A, 0, actionOutputChange, OUTPUT_SD_CARD_SAVVYCAN },
  //{ "Wireless Data", A, 0, actionOutputChange, OUTPUT_WIFI },
  { "Display Info", M, menuDisplay},
  { },                                                                // Menu Terminator
}; 

Menu menuRoot[]{
  // TODO  WARNING: H (header) option is not available in the horizontal menu
  { "Main Menu", H },
  { "Analyse CAN Bus Capacity", A, 0, actionOutputChange, OUTPUT_ANALYSE_CAN_BUS_RESULTS },
  //{ "CAN Settings", M, CANSettings},
  { "Output Type", M, menuOutput},
  { },                                                                // Menu Terminator
};

// Set and track the corrent menu being displayed
Menu* menu = menuRoot;                                                // Start with the top most menu

// Add MCP2515 Modules
MCP2515 mcp2515_1(CAN_BUS_0_CS_PIN);                                  // Create MCP2515 controller for High Speed CAN Bus 500kbps
MCP2515 mcp2515_2(CAN_BUS_1_CS_PIN);                                  // Create MCP2515 controller for Medium Speed CAN Bus 125kbps
struct can_frame frame;

// Add SD Card Serial Port
HardwareSerial SD_Port(SD_PORT_HARDWARE_SERIAL_NUMBER);               // Connect OpenLager to Hardware Serial Port specified


// Setup Serial, SD_CARD, CAN Bus, and Display
// Also check the Arduino CORE used to compile the code has been validated
void setup() {
  // Serial needs to be at least 2,000,000 baud otherwise lines will be dropped when outputting CAN lines to the standard serial output display.
  while (!Serial) { Serial.begin(STANDARD_SERIAL_OUTPUT_BAUD); delay(100); }

  // Display the Serial header
  bool versionMessageBoxRequired = false;
  Serial.printf("\nName        : OBD2_Interface_v2\nCreated     : Sep 2024\nAuthor      : Kevin Guest AKA TheBionicBone\n");
  Serial.printf("Program     : %s\n", TOP_MENU_PROGRAM_VERSION);
  Serial.printf("ESP-IDF     : %s\n",esp_get_idf_version());
  Serial.printf("Arduino Core: v%d.%d.%d\n", ESP_ARDUINO_VERSION_MAJOR, ESP_ARDUINO_VERSION_MINOR, ESP_ARDUINO_VERSION_PATCH);
  // Check the ESP32 Arduino Core used to compile the code has been validated 
  if (ESP_ARDUINO_VERSION != ESP_ARDUINO_VERSION_VAL(2, 0, 17)) {
    Serial.printf("\n\n");
    Serial.printf("This ESP Arduino Core Version has not been tested\n");
    Serial.printf("To ensure full compatibility use ESP32 Arduino Core v2.0.17\n");
    Serial.printf("***!!!*** PROCEED WITH CAUTION ***!!!***\n");
    versionMessageBoxRequired = true;
  }

  // Start SD Card
  SDCardStart(SD_CARD_ESP32_S3_TX_PIN);

  // Start the two CAN Bus, 500kbps for High Speed and 125kbps for the Medium Speed and both in ListenOnly Mode
  CANBusStart(mcp2515_1, CAN_500KBPS, 1); 
  CANBusStart(mcp2515_2, CAN_125KBPS, 1);

  // TFT_eSPI runs on HSPI bus, see Setup42_ILI9341_ESP32.h for pin definitions and more information
  TFT_Rectangle_ILI9341.init();
  pinMode(TFT_Rectangle_ILI9341_LEDPIN, OUTPUT);
  digitalWrite(TFT_Rectangle_ILI9341_LEDPIN, HIGH);

  // Calibrate the touch screen and retrieve the scaling factors, see Setup42_ILI9341_ESP32.h for more touch CS Pin definition
  TFT_Rectangle_ILI9341.setRotation(3);
  TFTRectangleILI9341TouchCalibrate();

  // Clear the display and reset the program header
  ClearDisplay();

  // Issue MessageBox about the ESP32 Arduino Core used to compile the code
  if (versionMessageBoxRequired) {
    MessageBox("WARNING", "To ensure full compatibility use ESP32 Arduino Core v2.0.17\n\n*!* USE WITH CAUTION *!*", BTN_OK);
  }

  pinMode(STOP_BUTTON_PIN, INPUT_PULLUP);
}


// At the end of each function we return here to reset and show the main root menu
void loop() {
  debugLoop("Called");

  //String returned = Keyboard.displayKeyboard();
  //Serial.print("Keyboard() Returned: "); Serial.println(returned);
  //while (true);

  //CANBusSettings(MENU_Y_OFFSET, MENU_BOLD_FONT);
  //while (true);

  // Reset required Global Control Variables
  // TODO - Review each Global Variable to see if it really needs to be Global
  totalCANReceiveTimeTimer = 0;                                       // Times how long we have been receiving CAN Frames
  numberOfCANFramesReceived[0] = 0;                                   // Counts the number of CAN Frames received
  numberOfCANFramesReceived[1] = 0;                                   // Counts the number of CAN Frames received
  outputFormat = false;                                               // Tracks the required output type

  // Ensure the SD Card is OK to eject
  SD_Port.flush();

  // Clear display and redraw the Title and Menu
  menu = menuRoot;                                                    // After the code function returns jump back to the root menu
  DrawVerticalMenu(MENU_Y_OFFSET, MENU_BOLD_FONT, MENU_FONT);
}



// Main Program Functions

void StartReadingCanBus() {
    /*
      For information
      As part of the MCP2515 CAN Bus Controllers are 3 Rx buffers in each, thus 3 for the 500kbps, and 3 for the 125kbps CAN Bus.
      500kbps CAN Bus will fill in a minimum period of 666us
      125kbps CAN bus will fill in a minimum period of 2664us
    */
  
  debugLoop("Called, outputFormat = %d", outputFormat);

  // Reset the display
  ClearDisplay();

  // Reset the uptimer for the logs
  upTimer = micros();

  uint16_t warning = 0;

  switch (outputFormat) {
  case OUTPUT_FORMAT_CANDRIVE:
    // CanDrive only supports one interface at a time, user must select one first
    interfaceNumber = MessageBox("Select Interface", "CanDrive only supports reading one CAN interface at a time. Please select which one.", BTN_CAN1 + BTN_CAN2);
    debugLoop("interfaceNumber = %d", interfaceNumber);

    TFT_Rectangle_ILI9341.setTextColor(TFT_GREEN);
    TFT_Rectangle_ILI9341.drawCentreString("Outputting CAN Bus data in CanDrive format", 160, 25, 2);
    TFT_Rectangle_ILI9341.drawCentreString("to the USB Port at 2,000,000 baud.", 160, 45, 2);
    TFT_Rectangle_ILI9341.drawCentreString("Connect PC with CanDrive running to see", 160, 65, 2);
    TFT_Rectangle_ILI9341.drawCentreString("and filter the Live CAN BUS Data.", 160, 85, 2);
    TFT_Rectangle_ILI9341.drawCentreString("Press STOP to end the output.", 160, 105, 2);
    TFT_Rectangle_ILI9341.setTextColor(TFT_BLACK);
    break;

  case OUTPUT_FORMAT_SAVVYCAN:
    interfaceNumber = MessageBox("Select Interface", "SavvyCAN can read single or both CAN interfaces. Please select an option.", BTN_CAN1 + BTN_CAN2 + BTN_CANBOTH);
    debugLoop("interfaceNumber = %d", interfaceNumber);

    TFT_Rectangle_ILI9341.setTextColor(TFT_GREEN);
    TFT_Rectangle_ILI9341.drawCentreString("Outputting CAN Bus data in SavvyCAN format", 160, 25, 2);
    TFT_Rectangle_ILI9341.drawCentreString("to the USB Port at 2,000,000 baud.", 160, 45, 2);
    TFT_Rectangle_ILI9341.drawCentreString("Connect PC with suitable recording program.", 160, 65, 2);
    TFT_Rectangle_ILI9341.drawCentreString("Press STOP to end the output.", 160, 85, 2);
    TFT_Rectangle_ILI9341.setTextColor(TFT_BLACK);
    break;

  case OUTPUT_SD_CARD_SAVVYCAN:
    interfaceNumber = MessageBox("Select Interface", "SavvyCAN can read single or both CAN interfaces. Please select an option.", BTN_CAN1 + BTN_CAN2 + BTN_CANBOTH);
    debugLoop("interfaceNumber = %d", interfaceNumber);
    displayPointInTime = MessageBox("Point In Time", "Display Point in Time Data (can limit data capture!) ?", BTN_YES + BTN_NO);
    debugLoop("displayPointInTime = %d", displayPointInTime);
    TFT_Rectangle_ILI9341.setTextColor(TFT_GREEN);
    TFT_Rectangle_ILI9341.drawCentreString("Saving CAN Bus data in SavvyCAN format", 160, 25, 2);
    TFT_Rectangle_ILI9341.drawCentreString("to SD Card at 2,000,000 baud.", 160, 45, 2);
    TFT_Rectangle_ILI9341.drawCentreString("Ensure the SD Card must be formatted using", 160, 65, 2);
    TFT_Rectangle_ILI9341.drawCentreString("FAT32 and be a good quality 32gb or less.", 160, 85, 2);
    TFT_Rectangle_ILI9341.drawCentreString("Press STOP to end the output.", 160, 105, 2);
    if (displayPointInTime == BTN_YES) {
      TFT_Rectangle_ILI9341.drawString("MS Date / Time:", 70, 145);
      TFT_Rectangle_ILI9341.drawString("    MS Car Age:", 70, 165);
      TFT_Rectangle_ILI9341.drawString("    MS 0x490D3:", 70, 185);
      TFT_Rectangle_ILI9341.drawString("    HS Car Age:", 70, 205);
      TFT_Rectangle_ILI9341.drawString("    HS 0x3D3D1:", 70, 225);
    }
    TFT_Rectangle_ILI9341.setTextColor(TFT_BLACK);
    break;

  case DISPLAY_POINT_IN_TIME:
    TFT_Rectangle_ILI9341.setTextColor(TFT_GREEN);
    TFT_Rectangle_ILI9341.drawCentreString("Display Reference Point in CAN Bus Time", 160, 20, 2);
    TFT_Rectangle_ILI9341.drawString("MS Date / Time:", 70, 45);
    TFT_Rectangle_ILI9341.drawString("    MS Car Age:", 70, 65);
    TFT_Rectangle_ILI9341.drawString("    MS 0x490D3:", 70, 85);
    TFT_Rectangle_ILI9341.drawString("    HS Car Age:", 70, 105);
    TFT_Rectangle_ILI9341.drawString("    HS 0x3D3D1:", 70, 125);
    interfaceNumber = CANBOTH;
    break;

  case DISPLAY_TESTING_DATA:
    // We probably want to send data so put into HS CAN into Normal Mode
#if ALLOW_SENDING_DATA_TO_CAN_BUS == 1
    warning = MessageBox("WARNING", "AKN, ERROR signals & Data Request Frames will be sent to the car. Use with caution. Do you want to continue ?", BTN_YES + BTN_NO);
    if (warning == BTN_NO) {
      return;
    }
    CANbusSetNormalMode(mcp2515_1);
#else
    MessageBox("INFORMATION", "Sending data is deactivated, send data requests will not be sent. See GitHub repository.", BTN_OK);
#endif
    
    TFT_Rectangle_ILI9341.setTextColor(TFT_GREEN);
    // Maximum of 10 Lines due to display size
    TFT_Rectangle_ILI9341.drawString("Battery Voltage:", 70, 20);
    TFT_Rectangle_ILI9341.drawString("Unused 2:", 70, 40);
    TFT_Rectangle_ILI9341.drawString("    Battery SoC:", 70, 60);
    TFT_Rectangle_ILI9341.drawString("  Also PID 4028:", 70, 80);
    TFT_Rectangle_ILI9341.drawString("Unused 5:", 70, 100);
    TFT_Rectangle_ILI9341.drawString("Unused 6:", 70, 120);
    TFT_Rectangle_ILI9341.drawString("Unused 7:", 70, 140);
    TFT_Rectangle_ILI9341.drawString("  Front Torque:", 70, 160);
    TFT_Rectangle_ILI9341.drawString("   Rear Torque:", 70, 180);
    TFT_Rectangle_ILI9341.drawString("  Module Deg C:", 70, 200);
    interfaceNumber = CANBOTH;
    break;

  default:
    break;
  }

  uint32_t Can1OverflowErrors = 0;
  uint32_t Can2OverflowErrors = 0;

  // Ensure the OverFlow Errors are cleared before we start
  CANBusOverFlowError(mcp2515_1);
  CANBusOverFlowError(mcp2515_2);

  while (true) {
    // Check the stop button
    if (digitalRead(STOP_BUTTON_PIN) == LOW) { 
      // Put back into ListenOnly Mode, it is the safest mode.
      CANbusSetListenOnlyMode(mcp2515_1);
      // Show a message box to report OverFlows "ONLY" if OverFlows occured.
      if (Can1OverflowErrors > 0 || Can2OverflowErrors > 2) {
        String helperString = "CAN Data was lost!\nCAN1 Overflows: " + String(Can1OverflowErrors) + "\nCAN2 Overflows: " + String(Can2OverflowErrors);
        MessageBox("Receiving Overflows", helperString.c_str(), BTN_OK);
      }
      return; 
    }

    
    // Check the 500kbps bus as priority over 125kbps because 500kbps is faster
    // and the buffers fill significantly more quickly
    if (CANBusCheckRecieved(mcp2515_1) && ((interfaceNumber & CAN1) || (interfaceNumber & CANBOTH))) {
      // Check for overflow errors, and clear them if necessary.
      if (CANBusOverFlowError(mcp2515_1)) Can1OverflowErrors++;
      if (CANBusReadCANData(mcp2515_1)) {
        debugLoop("mcp2515_1 read");
        CANFrameProcessing(1);
      }
    }
    // only check the 125kbps if there any no 500kbps messages in the MCP2515 buffers
    else if (CANBusCheckRecieved(mcp2515_2) && ((interfaceNumber & CAN2) || (interfaceNumber & CANBOTH))) {
      // Check for overflow errors, and clear them if necessary.
      if (CANBusOverFlowError(mcp2515_2)) Can2OverflowErrors++;
      if (CANBusReadCANData(mcp2515_2)) {
        debugLoop("mcp2515_2 read");
        CANFrameProcessing(2);
      }
    }
    
    //// Check the 125kbps bus as priority over 500kbps because 125kbps is slower so it is much quicker to clear the buffer
    //if (CANBusCheckRecieved(mcp2515_2) && ((interfaceNumber & CAN2) || (interfaceNumber & CANBOTH))) {
    //  // Check for overflow errors, and clear them if necessary.
    //  if (CANBusOverFlowError(mcp2515_2)) Can2OverflowErrors++;
    //  if (CANBusReadCANData(mcp2515_2)) {
    //    debugLoop("mcp2515_2 read");
    //    CANFrameProcessing(2);
    //  }
    //}
    //// only check the 500kbps if there any no 125kbps messages in the MCP2515 buffers
    //else if (CANBusCheckRecieved(mcp2515_1) && ((interfaceNumber & CAN1) || (interfaceNumber & CANBOTH))) {
    //  // Check for overflow errors, and clear them if necessary.
    //  if (CANBusOverFlowError(mcp2515_1)) Can1OverflowErrors++;
    //  if (CANBusReadCANData(mcp2515_1)) {
    //    debugLoop("mcp2515_1 read");
    //    CANFrameProcessing(1);
    //  }
    //}
  }
}


// Processes the CAN Frame and triggers the required output
// For Land Rover Freelander 2 set:
// whichBus = 0 for 500kpbs (High Speed) and 1 for 125kbps (Medium Speed)
void CANFrameProcessing(uint8_t whichCANBus) {
  
  // TODO - implement extend frame detection

  if (CANBusFirstRun) { 
    totalCANReceiveTimeTimer = micros();                              // Set the timer for calculating the CAN Frames per Second (cfps) 
    CANBusFirstRun = false; 
  }

  numberOfCANFramesReceived[whichCANBus]++;


  switch (outputFormat) {
  case OUTPUT_FORMAT_CANDRIVE:
    OutputFormatCanDrive(frame.can_id, frame.can_dlc, frame.data, whichCANBus);
      break;

  case OUTPUT_FORMAT_SAVVYCAN:
    outputFormatSavvyCAN(frame.can_id, frame.can_dlc, frame.data, whichCANBus);
    break;

  case OUTPUT_SD_CARD_SAVVYCAN:
    OutputSDCardSavvyCAN(frame.can_id, frame.can_dlc, frame.data, whichCANBus);
    break;

  case DISPLAY_POINT_IN_TIME:
    DisplayPointInTime(frame.can_id, frame.can_dlc, frame.data, whichCANBus, 35);
    break;

  case DISPLAY_TESTING_DATA:
    DisplayTestingData(frame.can_id, frame.can_dlc, frame.data, whichCANBus);
    break;

  default:
    break;
  }
  // Trigger the correct output here

  //SDCardCANFrameSavvyCANOutput(whichCANBus);
}




// SD Card Functions

// Starts the SD Card Device (OpenLager) @ 2,000,000 baud on ESP32-S3 TxPin
void SDCardStart(uint8_t TxPin) {
  debugLoop("Called");

  // Add a time out
  unsigned long timer = millis();

  // Due to the writing speed necessary I am using an STM32F411 based "OpenLager" (not to be confused with the slower "OpenLogger") 
  while (!SD_Port && (millis() - timer < 2500)) {
    SD_Port.begin(2000000, SERIAL_8N1, -1, TxPin);                    // Rx pin in not required, we will not use any OpenLager read options
    delay(500);
  }
  delay(1000);                                                        // For start up stability (corruption was noticed if we try to write immediately)
  if (SD_Port) { 
    debugLoop("SD Card writer initialised\n"); 
  }
  else {
    debugLoop("SD Card writer is not available\n");
  }
}


//  *!* Depreciated Code *!*
// Writes a CAN Frame to the SD Card Device (OpenLager) in SavvyCAN compatible format
void SDCardCANFrameSavvyCANOutput(uint8_t whichCANBus) {
  //if(sdCardFirstRun){ 
  //  SD_Port.printf("Time Stamp,ID,Extended,Bus,LEN,D1,D2,D3,D4,D5,D6,D7,D8\n"); 
  //  sdCardFirstRun = false; 
  //}

  //String helperString =
  //  String(micros()) + "," + String(frame.can_id, HEX) + ",false," + String(whichCANBus) + "," + String(frame.can_dlc) + ","
  //  + String(frame.data[0], HEX) + "," + String(frame.data[1], HEX) + "," + String(frame.data[2], HEX) + "," + String(frame.data[3], HEX) + ","
  //  + String(frame.data[4], HEX) + "," + String(frame.data[5], HEX) + "," + String(frame.data[6], HEX) + "," + String(frame.data[7], HEX) + "\n";
  //SD_Port.printf(helperString.c_str());
}



// MCP2515 CAN Controller Functions
  /****************************************************************************************  
   *  This coding has been designed this way so I can change the MCP2515 CAN BUS Library  *
   *  to a different one without changing the main code.                                  *
   ****************************************************************************************/

// Resets all CAN related Program Contol Variables
// for example for calculating CAN Frames per Second (cfps)
void CANBusResetControlVariables() {
  CANBusFirstRun = true;
  totalCANReceiveTimeTimer = 0;
  numberOfCANFramesReceived[0] = 0;
  numberOfCANFramesReceived[1] = 0;
}


void CANBusSettings(int16_t yOffset, GFXfont headerFont) {
  uint16_t xWidth = 0;
  
  // Clear the display and reset the program header
  ClearDisplay();

  TFT_Rectangle_ILI9341.setTextColor(TFT_GREEN);
  TFT_Rectangle_ILI9341.setTextDatum(TL_DATUM);

  // Quick test to enter a CAN Speed
  // Draw the menu header
  TFT_Rectangle_ILI9341.setFreeFont(&headerFont);
  xWidth = TFT_Rectangle_ILI9341.textWidth("CAN BUS Settings");
  TFT_Rectangle_ILI9341.drawString("CAN BUS Settings", (TFT_Rectangle_ILI9341.width() - xWidth) / 2, (yOffset - TFT_Rectangle_ILI9341.fontHeight()) / 2);

  // Draw available settings
  TFT_Rectangle_ILI9341.setTextFont(2);
  TFT_Rectangle_ILI9341.drawString("Module 1", 45, yOffset + 10);
  xWidth = TFT_Rectangle_ILI9341.drawString("Speed (kbps): ", 5, yOffset + 30);
  TFT_Rectangle_ILI9341.drawString(String(CANBusConf.Interface1_Speed), xWidth + 5, yOffset + 30);

  TFT_Rectangle_ILI9341.drawString("Module 2", 205, yOffset + 10);
  xWidth = TFT_Rectangle_ILI9341.drawString("Speed (kbps): ", 165, yOffset + 30);
  TFT_Rectangle_ILI9341.drawString(String(CANBusConf.Interface2_Speed), xWidth + 165, yOffset + 30);

}



// Starts one of the two MCP2515 CAN controllers
// For Land Rover Freelander 2 set:
// mcp2515_1 to 500kpbs (High Speed) in Mode 1 (ListenOnly)
// mcp2515_2 to 125kbps (Medium Speed) in Mode 1 (ListenOnly)
// CANMode = 0 for Normal and 1 for ListenOnly
void CANBusStart(MCP2515 CANBusModule, CAN_SPEED CANSpeed, uint8_t CANMode) {
  debugLoop("Called");
  
  while (CANBusModule.reset() != MCP2515::ERROR_OK) { delay(500); }
  debugLoop("MCP2515 Reset Successful");

  while (CANBusModule.setBitrate(CANSpeed, MCP_8MHZ) != MCP2515::ERROR_OK) { delay(500); }
  debugLoop("MCP2515 BitRate Successful");

  if (CANMode == 0) {
    CANbusSetNormalMode(CANBusModule);
  }
  else {                                                              
    CANbusSetListenOnlyMode(CANBusModule);
  }
}

// Set an MCP2515 CAN controller to Listen Only Mode
void CANbusSetListenOnlyMode(MCP2515 CANBusModule) {
  while (CANBusModule.setListenOnlyMode() != MCP2515::ERROR_OK) { delay(500); }
  debugLoop("MCP2515 Listen Only Mode Successful");
  CANBusResetControlVariables();
}

// Set an MCP2515 CAN controller to Normal Mode
void CANbusSetNormalMode(MCP2515 CANBusModule) {
  while (CANBusModule.setNormalMode() != MCP2515::ERROR_OK) { delay(500); }
  debugLoop("MCP2515 Normal Mode Successful");
  CANBusResetControlVariables();
}

// Checks the MCP2515 CAN controller RX Buffer for available data
bool CANBusCheckRecieved(MCP2515 CANBusModule) {
  return CANBusModule.checkReceive();
}

// Reads the MCP2515 CAN controller RX Buffer for available data
bool CANBusReadCANData(MCP2515 CANBusModule) {

  if (CANBusModule.readMessage(&frame) == MCP2515::ERROR_OK) {
    return true;
  }
  else {
    return false;
  }
}

// Send CAN Data
bool CANBusSendCANData(MCP2515 CANBusModule) {
#if ALLOW_SENDING_DATA_TO_CAN_BUS == 1
  CANBusModule.sendMessage(&frame);
  return true;
#else
  return false;
#endif
}

// Detect and Clear Receive Overflow Errors
bool CANBusOverFlowError(MCP2515 CANBusModule) {
  // Check for Receive Overflow Errors
  uint8_t errorFlags = CANBusModule.getErrorFlags();
  bool ret = false;
  if ((errorFlags & (1 << 6)) || (errorFlags & (1 << 7))) {
    //Overflow Error Flags Detected
    CANBusModule.clearRXnOVR();
    ret = true;
  }
  return ret;
}


// Writes satistics to Serial and SD Card outputs
void SDCardOutputResults() {
  debugLoop("Called");

  // Temporary - The results
  uint32_t totalCANReceiveTime = micros() - totalCANReceiveTimeTimer;
  debugLoop("Total Time = %d us", totalCANReceiveTime);
  debugLoop("500kbps Bus");
  debugLoop("numberOfFramesReceived = %d", numberOfCANFramesReceived[0]);
  debugLoop("Frames per Second = %.2f fps", (float)numberOfCANFramesReceived[0] / totalCANReceiveTime * 1000000);

  SD_Port.printf("Total Time = %d us\n", totalCANReceiveTime);
  SD_Port.printf("500kbps Bus\n");
  SD_Port.printf("numberOfFramesReceived = %d\n", numberOfCANFramesReceived[0]);
  SD_Port.printf("Frames per Second = %.2f fps\n", (float)numberOfCANFramesReceived[0] / totalCANReceiveTime * 1000000);

  // Calculate CAN Bus Capacity Used
  debugLoop("Capacity %% calculation data");
  debugLoop("numberOfFramesReceived = %d", numberOfCANFramesReceived[0]);
  debugLoop("totalReceiveTime = %d", totalCANReceiveTime);

  uint8_t percentageOfBusCapacity = 0.00;
  float timeTaken = 0.000000;                                         // define high precision floating point math
  timeTaken = (((float)totalCANReceiveTime / (float)1000000));        // time taken to read numberOfFramesReceived
  uint32_t bitsTx = 119 * numberOfCANFramesReceived[0];               // 119 bits (average based on small sample of FL2 live CAN Data).
  uint32_t bitRate = bitsTx / timeTaken;                              // bit rate used on the CAN bus
  percentageOfBusCapacity = ((float)bitRate / 500000) * 100;          // the percentage used of the CAN bus

  // For DEBUGGING
  debugLoop("timeTaken = %d", (uint32_t)timeTaken);
  debugLoop("bitsTx = %d", bitsTx); 
  debugLoop("bitRate = %d", bitRate);

  debugLoop("Capacity used %d%% (limited accuracy)\n", percentageOfBusCapacity);

  SD_Port.printf("Capacity used %d%% (limited accuracy)\n", percentageOfBusCapacity);

  debugLoop("125kbps Bus");
  debugLoop("numberOfFramesReceived = %d", numberOfCANFramesReceived[1]);
  debugLoop("Frames per Second = %.2f fps", (float)numberOfCANFramesReceived[1] / totalCANReceiveTime * 1000000);

  SD_Port.printf("125kbps Bus\n");
  SD_Port.printf("numberOfFramesReceived = %d\n", numberOfCANFramesReceived[1]);
  SD_Port.printf("Frames per Second = %.2f fps\n", (float)numberOfCANFramesReceived[1] / totalCANReceiveTime * 1000000);

  // Calculate CAN Bus Capacity Used
  debugLoop("Capacity %% calculation data");
  debugLoop("numberOfFramesReceived = %d", numberOfCANFramesReceived[1]);
  debugLoop("totalReceiveTime = %d", totalCANReceiveTime);

  percentageOfBusCapacity = 0.00;
  timeTaken = 0.000000;                                               // define high precision floating point math
  timeTaken = (((float)totalCANReceiveTime / (float)1000000));        // time taken to read numberOfFramesReceived
  bitsTx = 119 * numberOfCANFramesReceived[1];                        // 119 bits (average based on small sample of FL2 live CAN Data).
  bitRate = bitsTx / timeTaken;                                       // bit rate used on the CAN bus
  percentageOfBusCapacity = ((float)bitRate / 125000) * 100;          // the percentage used of the CAN bus

  debugLoop("timeTaken = %d", (uint32_t)timeTaken);
  debugLoop("bitsTx = %d", bitsTx);
  debugLoop("bitRate = %d", bitRate);

  debugLoop("Capacity used %d%% (limited accuracy)\n", percentageOfBusCapacity);

  SD_Port.printf("Capacity used %d%% (limited accuracy)\n", percentageOfBusCapacity);

  SD_Port.flush();
  SD_Port.end();
}


// Calibrate the touch screen and retrieve the scaling factors, see Setup42_ILI9341_ESP32.h for more touch CS Pin definition
// To recalibrate set REPEAT_CALIBRATION = true 
void TFTRectangleILI9341TouchCalibrate() {
  debugLoop("Called\n");
  
  // This calibration code came for the TFT_eSPI library example Keypad_240x320
  // My thanks to Bodmer for this code, copywrite acknowledged in the header and library folder

  uint16_t calData[5];
  uint8_t calDataOK = 0;

  // check file system exists
  if (!SPIFFS.begin()) {
    Serial.println("formatting file system");
    SPIFFS.format();
    SPIFFS.begin();
  }

  // check if calibration file exists and size is correct
  if (SPIFFS.exists(CALIBRATION_FILE)) {
    if (REPEAT_CALIBRATION)
    {
      // Delete if we want to re-calibrate
      SPIFFS.remove(CALIBRATION_FILE);
    }
    else
    {
      File f = SPIFFS.open(CALIBRATION_FILE, "r");
      if (f) {
        if (f.readBytes((char*)calData, 14) == 14)
          calDataOK = 1;
        f.close();
      }
    }
  }

  if (calDataOK && !REPEAT_CALIBRATION) {
    // calibration data valid
    TFT_Rectangle_ILI9341.setTouch(calData);
  }
  else {
    // data not valid so recalibrate
    TFT_Rectangle_ILI9341.fillScreen(TFT_BLACK);
    TFT_Rectangle_ILI9341.setCursor(20, 0);
    //TFT_Rectangle_ILI9341.setTextFont(2);
    //TFT_Rectangle_ILI9341.setTextSize(1);
    TFT_Rectangle_ILI9341.setTextColor(TFT_WHITE, TFT_BLACK);

    TFT_Rectangle_ILI9341.println("Touch corners as indicated");

    //TFT_Rectangle_ILI9341.setTextFont(1);
    TFT_Rectangle_ILI9341.println();

    if (REPEAT_CALIBRATION) {
      TFT_Rectangle_ILI9341.setTextColor(TFT_RED, TFT_BLACK);
      TFT_Rectangle_ILI9341.println("Set REPEAT_CAL to false to stop this running again!");
    }

    TFT_Rectangle_ILI9341.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

    TFT_Rectangle_ILI9341.setTextColor(TFT_GREEN, TFT_BLACK);
    TFT_Rectangle_ILI9341.println("Calibration complete!");

    // store data
    File f = SPIFFS.open(CALIBRATION_FILE, "w");
    if (f) {
      f.write((const unsigned char*)calData, 14);
      f.close();
    }
  }
}


// Clears the display and places the Program Name and Version at the top of the display
// Sets the rotation to 3, textColor to TFT_WHITE, TFT_LANDROVERGREEN, true
// Sets font to MENU_FONT & TextDatum(MC_DATUM)
// Requires FONT_2 to be loaded (header font)
void ClearDisplay() {
  debugLoop("Called\n");
  TFT_Rectangle_ILI9341.setRotation(3);
  TFT_Rectangle_ILI9341.fillScreen(TFT_LANDROVERGREEN);
  // Ensure the user is reminded if in DEBUG mode
#if DEBUG > 0
  TFT_Rectangle_ILI9341.setTextColor(TFT_RED, TFT_YELLOW, true);
  TFT_Rectangle_ILI9341.setTextDatum(TL_DATUM);
  TFT_Rectangle_ILI9341.drawString(String("DEBUG"), TFT_Rectangle_ILI9341.width() - 120, 0, TOP_MENU_FONT);
#endif
  TFT_Rectangle_ILI9341.setTextColor(TFT_WHITE, TFT_LANDROVERGREEN, true);
  TFT_Rectangle_ILI9341.setTextDatum(TL_DATUM);
  TFT_Rectangle_ILI9341.drawString(TOP_MENU_PROGRAM_NAME, 0, 0, TOP_MENU_FONT);
  TFT_Rectangle_ILI9341.setTextDatum(TR_DATUM);
  TFT_Rectangle_ILI9341.drawString(TOP_MENU_PROGRAM_VERSION, TFT_Rectangle_ILI9341.width(), 0, TOP_MENU_FONT);
  TFT_Rectangle_ILI9341.setTextDatum(MC_DATUM);
  TFT_Rectangle_ILI9341.setFreeFont(&MENU_FONT);
}


//  *!* Depreciated Code *!*
// Draw a menu along the top of the TFT Display (320 x 240) Rotation 3
// Menu options are drawn in a line across the top of the screen
// Contents based on the current menu defined by menu structures
void DrawHorizontalMenu(int16_t yOffset, GFXfont menuFont) {
  debugLoop("Called\n");

  // Clear the display and reset the program header
  ClearDisplay();

  // Determine the maximum number of characters in a button so we can calculate the button width correctly
  TFT_Rectangle_ILI9341.setFreeFont(&menuFont);                       // Must set menu font for following calculations
  uint8_t maxChars = 0;
  uint8_t menuButtons = 0;
  while (menu[menuButtons].text) {
    if (maxChars < String(menu[menuButtons].text).length()) maxChars = String(menu[menuButtons].text).length();
    menuButtons++;
  }
  if (maxChars < 10) maxChars = 10;                                   // Draw a minimum button size so user can easily select it

  // Calculate the basic button positions
  uint16_t fontWidth = 10; // take a default value for now
  uint16_t yButtonMiddle = TFT_Rectangle_ILI9341.fontHeight() + yOffset;
  uint16_t xButtonWidth = fontWidth * maxChars;
  uint16_t yButtonHeight = TFT_Rectangle_ILI9341.fontHeight() * 1;

  // Draw the menu buttons
  TFT_Rectangle_ILI9341.setFreeFont(&menuFont);
  TFT_eSPI_Button btnMenu[5];                                         // Maximum 5 buttons on a horizontal menu
  const char* btnText[] = { "", "", "", "", "" };                     // Maximum 5 buttons on a horizontal menu
  char handler[1] = "";
  uint8_t menuBtnStartPos = 0;
  menuButtons = 0;                                                    // Reset the number of buttons that will be active on the new menu
  while (menu[menuButtons].text) {
    btnText[menuButtons] = menu[menuButtons].text;                    // Must capture btnText for the ProcessButtons() function
    btnMenu[menuButtons].initButton(&TFT_Rectangle_ILI9341,
      (menuButtons * xButtonWidth) + (menuButtons * 5) + (xButtonWidth / 2),
      yButtonMiddle,
      xButtonWidth,
      yButtonHeight,
      TFT_YELLOW, TFT_BLUE, TFT_YELLOW, handler, 1);                  // initButton limits the amount of text drawn, draw text in the drawButton() function
    btnMenu[menuButtons].drawButton(false, menu[menuButtons].text);   // Specifiy the text for the button because initButton will not display the full text length
    btnMenu[menuButtons].press(false);                                // Because I am reusing buttons it is important to tell the button it is NOT pressed
    menuButtons++;                                                    // Add the button to the Global Variable that tracks the number of active buttons 
  }
  ProcessButtons(btnMenu, btnText, MENU, menuButtons, menuBtnStartPos, true);
}


// Draw a menu in middle of the TFT Display (320 x 240) Rotation 3
// Menu options are drawn in a line down the screen and presented as buttons
// Contents based on the current menu defined by menu structures
void DrawVerticalMenu(int16_t yOffset, GFXfont headerFont, GFXfont menuFont) {
  debugLoop("Called\n");
  
  // Clear the display and reset the program header
  ClearDisplay();

  // Determine the maximum number of characters in a button so we can calculate the 
  // button width correctly and starting position of the buttons in the menu structure
  TFT_Rectangle_ILI9341.setFreeFont(&menuFont);                       // Must set menu font for following calculations
  uint8_t maxChars = 0;
  uint8_t menuPosition = 0;
  uint8_t menuButtons = 0;
  uint8_t menuBtnStartPos = 99;
  while (menu[menuPosition].text) {
    if (A == menu[menuPosition].action || M == menu[menuPosition].action) {
      if (maxChars < String(menu[menuPosition].text).length()) maxChars = String(menu[menuPosition].text).length();
      if (menuBtnStartPos == 99) menuBtnStartPos = menuPosition;      // Capture the position of the first button in the menu structure
      menuButtons++;
    }
    menuPosition++;
  }
  if (maxChars < 10) maxChars = 10;                                   // Minimum button size so user can easily select it

  // Calculate the basic button positions
  uint16_t fontWidth = 10; // take a default value for now
  uint16_t xButtonMiddle = TFT_Rectangle_ILI9341.width() / 2;
  uint16_t yButtonMiddle = TFT_Rectangle_ILI9341.fontHeight() * 1.8;
  uint16_t xButtonWidth = fontWidth * maxChars;
  uint16_t yButtonHeight = TFT_Rectangle_ILI9341.fontHeight() * 1.30;

  // Draw the menu buttons
  TFT_Rectangle_ILI9341.setFreeFont(&menuFont);
  TFT_eSPI_Button btnMenu[5];                                         // Maximum 5 buttons on a verticle menu
  const char* btnText[] = { "", "", "", "", ""};                      // Maximum 5 buttons on a verticle menu
  char handler[1] = "";
  menuPosition = 0;
  menuButtons = 0;

  // Reset the Global Variable that tracks the number of buttons that will be active on the new menu
  while (menu[menuPosition].text) {
    if (H == menu[menuPosition].action) {
      // Draw the menu header
      TFT_Rectangle_ILI9341.setFreeFont(&headerFont);
      TFT_Rectangle_ILI9341.drawString(menu[menuPosition].text, xButtonMiddle, (yOffset - TFT_Rectangle_ILI9341.fontHeight()) / 2);
      TFT_Rectangle_ILI9341.setFreeFont(&menuFont);
    }
    else if (A == menu[menuPosition].action || M == menu[menuPosition].action) {
      // Draw the action button
      btnText[menuButtons] = menu[menuPosition].text;                 // Must capture btnText for the ProcessButtons() function
      btnMenu[menuButtons].initButton(&TFT_Rectangle_ILI9341,
        xButtonMiddle,
        menuButtons * yButtonMiddle + yOffset,
        xButtonWidth,
        yButtonHeight,
        TFT_YELLOW, TFT_BLUE, TFT_YELLOW, handler, 1);                // initButton limits the amount of text drawn, draw text in the drawButton() function
      btnMenu[menuButtons].drawButton(false, menu[menuPosition].text);// Specifiy the text for the button because initButton will not display the full text length
      btnMenu[menuButtons].press(false);                              // Because I am reusing buttons it is important to tell the button it is NOT pressed
      menuButtons++;                                                  // Add the button to the Global Variable that tracks the number of active buttons
    }
    menuPosition++;
  }
  ProcessButtons(btnMenu, btnText, MENU, menuButtons, menuBtnStartPos, true);
}


// Checks if any active buttons on the display have been pressed
// Once called it will wait for a button to be pressed
// type: 0 = Menu, 1 = MessageBox
// NOTE: Overload automatically sets wait (for button press) = true
uint8_t ProcessButtons(TFT_eSPI_Button btnMenu[], const char* btnText[], uint8_t type, uint8_t numberOfButtons) {          // overload
  debugLoop("Called (without menuBtnStartPos overload)\n");
  uint8_t result = ProcessButtons(btnMenu, btnText, type, numberOfButtons, 0, true);
  return result;
}

// Checks if any active buttons on the display have been pressed
// Once called it will wait for a button to be pressed
// type: 0 = Menu, 1 = MessageBox
// wait (for button press): true or false
// Returns: number of button pressed or 99 if no button pressed
uint8_t ProcessButtons(TFT_eSPI_Button btnMenu[], const char* btnText[], uint8_t type, uint8_t numberOfButtons, uint8_t menuBtnStartPos, bool wait) {
  debugLoop("Called (with menuBtnStartPos overload)");
  debugLoop("wait = %d\n", wait);
  
  do {   // for now create an endless loop until a button is pressed

    uint16_t t_x = 0, t_y = 0;                                        // To store the touch coordinates

    // Pressed will be set true is there is a valid touch on the screen
    bool pressed = TFT_Rectangle_ILI9341.getTouch(&t_x, &t_y);

    // Check if any key coordinate boxes contain the touch coordinates
    for (uint8_t btnCounter = 0; btnCounter < numberOfButtons; btnCounter++) {
      if (pressed && btnMenu[btnCounter].contains(t_x, t_y)) {
        btnMenu[btnCounter].press(true);                              // tell the button it is pressed
      }
      else {
        btnMenu[btnCounter].press(false);                             // tell the button it is NOT pressed
      }
    }

    // Check if any key has changed state
    for (uint8_t btnCounter = 0; btnCounter < numberOfButtons; btnCounter++) {

      TFT_Rectangle_ILI9341.setFreeFont(&MENU_FONT);

      if (btnMenu[btnCounter].justPressed()) {
        btnMenu[btnCounter].drawButton(true, String(btnText[btnCounter]));    // draw inverted
        delay(100);                                                   // UI debouncing
      }

      if (btnMenu[btnCounter].justReleased()) {
        btnMenu[btnCounter].drawButton(false, String(btnText[btnCounter]));   // draw normal

        if (type == MENU) {
          ProcessMenu(btnCounter, menuBtnStartPos);                   // Jump to new menu
          return 99;
        }
        if (type == MESSAGE_BOX) return btnCounter;                   // Return message button pressed
        if (type == DISPLAY_BUTTON) return btnCounter + menuBtnStartPos; // Return display button pressed

        Serial.printf("ERROR: Button type not handled correctly, type = %d\n", type);
        return btnCounter;
      }
    }
  } while (wait);
  return 99;
}


// Controls the move to a new menu that will be displayed
// btnNumber = the button the user pressed
// menuBtnStartPos = the postion of the first menu option in the menu structure
void ProcessMenu(uint8_t btnNumber, uint8_t menuBtnStartPos) {
  debugLoop("Called\n");
  if (M == menu[btnNumber + menuBtnStartPos].action) {                // User selection required another menu
    menu = menu[btnNumber + menuBtnStartPos].menu;
    DrawVerticalMenu(MENU_Y_OFFSET, MENU_BOLD_FONT, MENU_FONT);
  }
  else if (A == menu[btnNumber + menuBtnStartPos].action) {           // User selection calls a code function
    menu[btnNumber + menuBtnStartPos].func(menu[btnNumber + menuBtnStartPos].arg);
  }
}


// Displays a message box on the display and allows the user to respond
// Use options to specify buttons, i.e. BTN_OK + BTN_IGNORE + BTN_CANCEL
uint16_t MessageBox(const char* title, const char* message, uint16_t options) {
  debugLoop("Called\n");
  uint16_t result = 0;
  uint16_t boxX = TFT_Rectangle_ILI9341.width() * 0.05;
  uint16_t boxY = TFT_Rectangle_ILI9341.height() * 0.2 + TOP_MENU_Y_OFFSET;
  uint16_t boxWidth = TFT_Rectangle_ILI9341.width() * 0.9;
  uint16_t boxHeight = TFT_Rectangle_ILI9341.height() * 0.6;
  uint16_t lineY = boxY;

  // Set up the display
  ClearDisplay();
  TFT_Rectangle_ILI9341.setTextColor(TFT_YELLOW, TFT_BLUE, true);

  // Draw the message box
  TFT_Rectangle_ILI9341.drawRect(boxX - 1, boxY - 1, boxWidth + 2, boxHeight + 2, TFT_YELLOW);
  TFT_Rectangle_ILI9341.fillRect(boxX, boxY, boxWidth, boxHeight, TFT_BLUE);

  // Draw the message title
  lineY += TFT_Rectangle_ILI9341.fontHeight() * 0.5;
  TFT_Rectangle_ILI9341.setFreeFont(&MENU_BOLD_FONT);
  TFT_Rectangle_ILI9341.drawString(String(title), TFT_Rectangle_ILI9341.width() / 2, lineY);
  TFT_Rectangle_ILI9341.setFreeFont(&MENU_FONT);
  lineY += TFT_Rectangle_ILI9341.fontHeight() * 1.5;

  // Split the message into lines and draw
  const uint16_t maxLineLength = 30;
  char lineBuffer[maxLineLength + 1];                                 // +1 for the null-terminator
  uint16_t lineLength = 0;
  const char* current = message;

  while (*current != '\0') {
    lineLength = 0;
    // Fill the buffer with characters until reaching the maximum length or newline
    while (lineLength < maxLineLength && *current != '\n' && *current != '\0') {
      lineBuffer[lineLength++] = *current++;
    }
    // Handle splitting lines at spaces if necessary
    if (lineLength == maxLineLength && *current != '\0' && *current != ' ' && *current != '\n') {
      // Walk backwards to find a space
      while (lineLength > 0 && lineBuffer[lineLength - 1] != ' ') {
        lineLength--;
        current--;
      }
    }
    // Null-terminate the string and print it
    lineBuffer[lineLength] = '\0';
    TFT_Rectangle_ILI9341.drawString(String(lineBuffer), TFT_Rectangle_ILI9341.width() / 2, lineY);
    lineY += TFT_Rectangle_ILI9341.fontHeight() * 0.8;
    // Skip the newline characters if present
    if (*current == '\n') {
      current++;
    }
    // Skip any leading spaces on the next line
    while (*current == ' ') {
      current++;
    }
  }

  // Work out how many buttons we will need and their text
  uint8_t numberOfOptionButtons = 0;
  const char* messageBtnText[3] = { "","","" };
  uint16_t messageButtonPos[3] = { 0,0,0 };

  if (options & BTN_OK) { messageBtnText[numberOfOptionButtons] = "OK"; numberOfOptionButtons++; }
  if (options & BTN_IGNORE) { messageBtnText[numberOfOptionButtons] = "IGNORE"; numberOfOptionButtons++; }
  if (options & BTN_CANCEL) { messageBtnText[numberOfOptionButtons] = "CANCEL"; numberOfOptionButtons++; }
  if (options & BTN_CAN1) { messageBtnText[numberOfOptionButtons] = "CAN 1"; numberOfOptionButtons++; }
  if (options & BTN_CAN2) { messageBtnText[numberOfOptionButtons] = "CAN 2"; numberOfOptionButtons++; }
  if (options & BTN_CANBOTH) { messageBtnText[numberOfOptionButtons] = "BOTH"; numberOfOptionButtons++; }
  if (options & BTN_YES) { messageBtnText[numberOfOptionButtons] = "YES"; numberOfOptionButtons++; }
  if (options & BTN_NO) { messageBtnText[numberOfOptionButtons] = "NO"; numberOfOptionButtons++; }

  debugLoop("MessageBox numberOfOptionButtons = %d", numberOfOptionButtons);

  // Draw the buttons
  uint8_t menuButtons = 0;
  TFT_eSPI_Button btnMenu[3];                                         // Maximum 3 buttons on a message box
  const char* btnText[] = { "", "", "" };                             // Maximum 3 buttons on a message box
  char handler[1] = "";
  uint16_t xButtonWidth = (boxWidth / 3) * 0.95;
  uint16_t yButtonHeight = TFT_Rectangle_ILI9341.fontHeight() * 1.2;
  uint16_t yButtonMiddle = boxY + boxHeight - yButtonHeight * 0.75;
  if (numberOfOptionButtons == 1) {
    messageButtonPos[0] = TFT_Rectangle_ILI9341.width() / 2;
  }
  else if (numberOfOptionButtons == 2) {
    messageButtonPos[0] = (TFT_Rectangle_ILI9341.width() / 2) - xButtonWidth - 5;
    messageButtonPos[1] = (TFT_Rectangle_ILI9341.width() / 2) + xButtonWidth + 5;
  }
  else if (numberOfOptionButtons == 3) {
    messageButtonPos[0] = (TFT_Rectangle_ILI9341.width() / 2) - xButtonWidth - 5;
    messageButtonPos[1] = TFT_Rectangle_ILI9341.width() / 2;
    messageButtonPos[2] = (TFT_Rectangle_ILI9341.width() / 2) + xButtonWidth + 5;
  }

  for (uint16_t i = 0; i < numberOfOptionButtons; i++) {
    btnText[menuButtons] = messageBtnText[menuButtons];               // Must capture btnText for the ProcessButtons() function
    btnMenu[menuButtons].initButton(&TFT_Rectangle_ILI9341,
      messageButtonPos[i],
      yButtonMiddle,
      xButtonWidth,
      yButtonHeight,
      TFT_YELLOW, TFT_BLUE, TFT_YELLOW, handler, 1);                  // initButton limits the amount of text drawn, draw text in the drawButton() function
    btnMenu[menuButtons].drawButton(false, messageBtnText[menuButtons]);// Specifiy the text for the button because initButton will not display the full text length
    btnMenu[menuButtons].press(false);                                // Because I am reusing buttons it is important to tell the button it is NOT pressed
    menuButtons++;
  }

  result = ProcessButtons(btnMenu, btnText, MESSAGE_BOX, menuButtons);

  // Clear the display and reset the program header
  ClearDisplay();

  // Evaluate the user choice to the MESSAGE_BOX enum values
  // TODO link enum to result by value rather than button text, but not sure how :)
  const char* helper0 = "OK";
  const char* helper1 = "IGNORE";
  const char* helper2 = "CANCEL";
  const char* helper3 = "CAN 1";
  const char* helper4 = "CAN 2";
  const char* helper5 = "BOTH";
  const char* helper6 = "YES";
  const char* helper7 = "NO";

  if (messageBtnText[result] == helper0) result = BTN_OK;
  else if (messageBtnText[result] == helper1) result = BTN_IGNORE;
  else if (messageBtnText[result] == helper2) result = BTN_CANCEL;
  else if (messageBtnText[result] == helper3) result = BTN_CAN1;
  else if (messageBtnText[result] == helper4) result = BTN_CAN2;
  else if (messageBtnText[result] == helper5) result = BTN_CANBOTH;
  else if (messageBtnText[result] == helper6) result = BTN_YES;
  else if (messageBtnText[result] == helper7) result = BTN_NO;

  debugLoop("MessageBox result = %d (MESSAGE_BOX_BUTONS)", result);

  return result;
}


// Performs analysis on both CAN Interfaces to determine if the OBD2_Interface can sucessfully read both at the same time
void OutputAnalyseCANBusResults() {

  /*
    For information
    As part of the MCP2515 CAN Bus Controllers are 3 Rx buffers in each, thus 3 for the 500kbps, and 3 for the 125kbps CAN Bus.
    500kbps CAN Bus will fill in a minimum period of 666us
    125kbps CAN bus will fill in a minimum period of 2664us
  */

#define MEASURE_TIME 10000000                                         // 10000000 for live, 1000000 to TESTING
#if MEASURE_TIME != 10000000
  #warning "OutputAnalyseCANBusResults() IS NOT EQUAL to 10000000"
#endif

  debugLoop("Called");
  uint8_t result = MessageBox("Analyse CAN Bus Capacity", "Ensure the OBD2 device is connected to the car with the engine running.", BTN_OK + BTN_CANCEL);

  debugLoop("MessageBox Returned Button %d", result);

  if (result == BTN_OK) {
    numberOfCANFramesReceived[0] = 0;
    numberOfCANFramesReceived[1] = 0;
    uint8_t   percentageOfBusCapacity[2]{ 0,0 };
    uint16_t  cfps[2] = { 0,0 };
    uint32_t  totalCANReceiveTime = 0;
    CANBusFirstRun = false;

    // Clear the display and reset the program header
    ClearDisplay();

    TFT_Rectangle_ILI9341.setTextFont(2);
    TFT_Rectangle_ILI9341.setTextColor(TFT_BLACK);
    TFT_Rectangle_ILI9341.setTextDatum(TL_DATUM);

    // Draw Results table
    uint16_t tableX = 20;
    uint16_t tableY = 100;
    uint16_t tableW = 280;
    uint16_t tableH = 80;
    uint16_t tableFontH = TFT_Rectangle_ILI9341.fontHeight();
    uint16_t tableFontW = TFT_Rectangle_ILI9341.textWidth("A");
    // Draw table
    TFT_Rectangle_ILI9341.drawRect(tableX - 1, tableY - 1, tableW + 2, tableH + 2, TFT_LIGHTGREY);
    TFT_Rectangle_ILI9341.fillRect(tableX, tableY, tableW, tableH, TFT_GREEN);
    // Horizontal Lines
    TFT_Rectangle_ILI9341.drawLine(tableX, tableY + tableFontH + 2, tableX + tableW, tableY + tableFontH + 2, TFT_LIGHTGREY);
    TFT_Rectangle_ILI9341.drawLine(tableX, tableY + tableFontH + 22, tableX + tableW, tableY + tableFontH + 22, TFT_LIGHTGREY);
    TFT_Rectangle_ILI9341.drawLine(tableX, tableY + tableFontH + 42, tableX + tableW, tableY + tableFontH + 42, TFT_LIGHTGREY);
    // Vertical Lines
    TFT_Rectangle_ILI9341.drawLine(tableX + (tableFontW * 7), tableY, tableX + (tableFontW * 7), tableY + tableH, TFT_LIGHTGREY);
    TFT_Rectangle_ILI9341.drawLine(tableX + (tableFontW * 17), tableY, tableX + (tableFontW * 17), tableY + tableH, TFT_LIGHTGREY);
    TFT_Rectangle_ILI9341.drawLine(tableX + (tableFontW * 26), tableY, tableX + (tableFontW * 26), tableY + tableH, TFT_LIGHTGREY);

    TFT_Rectangle_ILI9341.drawString("Results", tableX + 2, tableY);
    TFT_Rectangle_ILI9341.drawCentreString("Capacity*", tableX + (tableFontW * 12), tableY, 2);
    TFT_Rectangle_ILI9341.drawCentreString("Single", tableX + (tableFontW * 22), tableY, 2);
    TFT_Rectangle_ILI9341.drawCentreString("Dual", tableX + (tableFontW * 31), tableY, 2);

    TFT_Rectangle_ILI9341.drawString("500kbps", tableX + 2, tableY + 20);
    TFT_Rectangle_ILI9341.drawString("125kbps", tableX + 2, tableY + 40);
    TFT_Rectangle_ILI9341.drawString("Passed", tableX + 2, tableY + 60);

    TFT_Rectangle_ILI9341.setTextColor(TFT_GREEN);
    TFT_Rectangle_ILI9341.drawCentreString("Performs analysis on both CAN Interfaces", 160, 25, 2);
    TFT_Rectangle_ILI9341.drawCentreString("to determine if the OBD2_Interface can", 160, 45, 2);
    TFT_Rectangle_ILI9341.drawCentreString("sucessfully read both at the same time.", 160, 65, 2);
    TFT_Rectangle_ILI9341.drawString("* Calculated as 119 bits per CAN Frame", 2, TFT_Rectangle_ILI9341.height() - 10, 1);
    TFT_Rectangle_ILI9341.setTextColor(TFT_BLACK);

    // Check CAN Interface 0 CAN frames per second (cfps)
    totalCANReceiveTimeTimer = micros();                              // Set the timer for calculating the CAN Frames per Second (cfps) 
    while (micros() - totalCANReceiveTimeTimer < MEASURE_TIME) {          // Measure for 10 seconds
      if (CANBusCheckRecieved(mcp2515_1)) {
        if (CANBusReadCANData(mcp2515_1)) {
          CANFrameProcessing(0);
        }
      }
    }
    // Calculate cfps
    totalCANReceiveTime = micros() - totalCANReceiveTimeTimer;
    cfps[0] = (float)numberOfCANFramesReceived[0] / totalCANReceiveTime * 1000000;
    // Calculate %
    float timeTaken = 0.000000;                                         // define high precision floating point math
    timeTaken = (((float)totalCANReceiveTime / (float)1000000));        // time taken to read numberOfFramesReceived
    uint64_t bitsTx = 119 * numberOfCANFramesReceived[0];               // 119 bits (average based on small sample of FL2 live CAN Data).
    uint64_t bitRate = bitsTx / timeTaken;                              // bit rate used on the CAN bus
    percentageOfBusCapacity[0] = ((float)bitRate / 500000) * 100;          // the percentage used of the CAN bus
    // Update table
    TFT_Rectangle_ILI9341.drawCentreString(String(percentageOfBusCapacity[0]) + "%", tableX + (tableFontW * 12), tableY + 20, 2);
    TFT_Rectangle_ILI9341.drawCentreString(String(cfps[0]) + "cfps", tableX + (tableFontW * 22), tableY + 20, 2);


    // Check CAN Interface 1 CAN frames per second (cfps)
    totalCANReceiveTimeTimer = micros();                              // Set the timer for calculating the CAN Frames per Second (cfps) 
    while (micros() - totalCANReceiveTimeTimer < MEASURE_TIME) {          // Measure for 10 seconds
      if (CANBusCheckRecieved(mcp2515_2)) {
        if (CANBusReadCANData(mcp2515_2)) {
          CANFrameProcessing(1);
        }
      }
    }
    // Calculate cfps
    totalCANReceiveTime = micros() - totalCANReceiveTimeTimer;
    cfps[1] = (float)numberOfCANFramesReceived[1] / totalCANReceiveTime * 1000000;
    // Calculate %
    timeTaken = 0.000000;                                               // define high precision floating point math
    timeTaken = (((float)totalCANReceiveTime / (float)1000000));        // time taken to read numberOfFramesReceived
    bitsTx = 119 * numberOfCANFramesReceived[1];                    // 119 bits (average based on small sample of FL2 live CAN Data).
    bitRate = bitsTx / timeTaken;                                    // bit rate used on the CAN bus
    percentageOfBusCapacity[1] = ((float)bitRate / 125000) * 100;          // the percentage used of the CAN bus
    // Update table
    TFT_Rectangle_ILI9341.drawCentreString(String(percentageOfBusCapacity[1]) + "%", tableX + (tableFontW * 12), tableY + 40, 2);
    TFT_Rectangle_ILI9341.drawCentreString(String(cfps[1]) + "cfps", tableX + (tableFontW * 22), tableY + 40, 2);

    // If cfps[] is Zero then we should fail these tests (i.e. no CAN Data detected)
    if (cfps[0] != 0) {
      TFT_Rectangle_ILI9341.drawCentreString("Yes", tableX + (tableFontW * 12), tableY + 60, 2);
    }
    else {
      TFT_Rectangle_ILI9341.drawCentreString("No", tableX + (tableFontW * 12), tableY + 60, 2);
    }
    if (cfps[1] != 0) {
      TFT_Rectangle_ILI9341.drawCentreString("Yes", tableX + (tableFontW * 22), tableY + 60, 2);
    }
    else {
      TFT_Rectangle_ILI9341.drawCentreString("No", tableX + (tableFontW * 22), tableY + 60, 2);
    }

    // Check both CAN Interfaces at the same time for CAN frames per second (cfps)
    numberOfCANFramesReceived[0] = 0;
    numberOfCANFramesReceived[1] = 0;
    totalCANReceiveTimeTimer = micros();                              // Set the timer for calculating the CAN Frames per Second (cfps) 
    while (micros() - totalCANReceiveTimeTimer < MEASURE_TIME) {          // Measure for 10 seconds
      // Check the 500kbps bus as priority over 125kbps because 500kbps is faster and the buffers fill significantly more quickly
      if (CANBusCheckRecieved(mcp2515_1)) {
        if (CANBusReadCANData(mcp2515_1)) {
          CANFrameProcessing(0);
        }
      }
      // only check the 125kbps if there any no 500kbps messages in the MCP2515 buffers
      else if (CANBusCheckRecieved(mcp2515_2)) {
        if (CANBusReadCANData(mcp2515_2)) {
          CANFrameProcessing(1);
        }
      }
    }
    totalCANReceiveTime = micros() - totalCANReceiveTimeTimer;

    // Compare the results for CAN Interface 0
    bool passed = true;

    debugLoop("passed = %d", passed);

    uint16_t cfpsCompare = (float)numberOfCANFramesReceived[0] / totalCANReceiveTime * 1000000;

    debugLoop("numberOfCANFramesReceived[0] = %d", numberOfCANFramesReceived[0]);
    debugLoop("totalCANReceiveTime = %dus", totalCANReceiveTime);
    debugLoop("cfpsCompare = %d", cfpsCompare);
    debugLoop("cfps[0] = %d (%d%%)", cfps[0], percentageOfBusCapacity[0]);

    // If no CAN Bus data then fail the test
    if (cfps[0] == 0 || cfps[1] == 0) {
      passed = false;
      TFT_Rectangle_ILI9341.setTextColor(TFT_RED, TFT_YELLOW, true);
      TFT_Rectangle_ILI9341.drawString("ERROR: *!* NO CAN BUS DATA RECEIVED", 2, TFT_Rectangle_ILI9341.height() - 30, 2);
      TFT_Rectangle_ILI9341.setTextColor(TFT_BLACK, TFT_GREEN, true);
    }

    // If cfps for Dual CAN Bus reading is ont within 1% of the cfps for Single CAN Bus then fail the test 
    if ((cfpsCompare < cfps[0] * 0.99) || (cfpsCompare > cfps[0] * 1.01)) { passed = false; }
    debugLoop("passed = %d", passed);

    TFT_Rectangle_ILI9341.drawCentreString(String(cfpsCompare) + "cfps", tableX + (tableFontW * 31), tableY + 20, 2);

    // Compare the results for CAN Interface 1
    cfpsCompare = (float)numberOfCANFramesReceived[1] / totalCANReceiveTime * 1000000;

    debugLoop("numberOfCANFramesReceived[1] = %d", numberOfCANFramesReceived[1]);
    debugLoop("totalCANReceiveTime = %dus", totalCANReceiveTime);
    debugLoop("cfpsCompare = %d", cfpsCompare);
    debugLoop("cfps[1] = %d (%d%%)", cfps[1], percentageOfBusCapacity[1]);

    // If cfps for Dual CAN Bus reading is ont within 1% of the cfps for Single CAN Bus then fail the test 
    if ((cfpsCompare < cfps[1] * 0.99) || (cfpsCompare > cfps[1] * 1.01)) { passed = false; }
    debugLoop("passed = %d", passed);

    TFT_Rectangle_ILI9341.drawCentreString(String(cfpsCompare) + "cfps", tableX + (tableFontW * 31), tableY + 40, 2);
    if (passed) {
      TFT_Rectangle_ILI9341.drawCentreString("Yes", tableX + (tableFontW * 31), tableY + 60, 2);
    }
    else {
      TFT_Rectangle_ILI9341.drawCentreString("No", tableX + (tableFontW * 31), tableY + 60, 2);
    }

    // Create an OK button to exit
    TFT_Rectangle_ILI9341.setFreeFont(&MENU_FONT);                    // Set the normal button font
    char handler[1] = "";
    uint16_t xButtonWidth = TFT_Rectangle_ILI9341.textWidth("A") * 3;
    uint16_t yButtonHeight = TFT_Rectangle_ILI9341.fontHeight() * 1.2;
    uint16_t xButtonMiddle = TFT_Rectangle_ILI9341.width() - (xButtonWidth / 2);
    uint16_t yButtonMiddle = TFT_Rectangle_ILI9341.height() - (yButtonHeight / 2);
    // TODO - remove messageBtnText
    TFT_eSPI_Button btnMenu[1];                                         // Maximum 1 button on this display
    const char* btnText[1] = { "OK" };
    btnMenu[0].initButton(&TFT_Rectangle_ILI9341,
      xButtonMiddle,
      yButtonMiddle,
      xButtonWidth,
      yButtonHeight,
      TFT_YELLOW, TFT_BLUE, TFT_YELLOW, handler, 1);                  // initButton limits the amount of text drawn, draw text in the drawButton() function
    btnMenu[0].drawButton(false, btnText[0]);                         // Specifiy the text for the button because initButton will not display the full text length
    btnMenu[0].press(false);                                          // Because I am reusing buttons it is important to tell the button it is NOT pressed

    result = ProcessButtons(btnMenu, btnText, MESSAGE_BOX, 1);
    return;                                                           // Back to root menu
  }
  else if (result == BTN_CANCEL) {
    return;                                                           // Back to root menu
  }

  while (true) { Serial.printf("OutputAnalyseCANBusResults MessageBox result"); delay(1000); }
}


void OutputFormatCanDrive(ulong rxId, uint8_t len, uint8_t rxBuf[], uint8_t MCP2515number) {
  // canDrive Format - Validated as working Sep 2024
  // ID,Remote Transmission Request(RTR),Extended(IDE),D0,D1,D2,D3,D4,D5,D6,D7
  // 151,0,0,025B8151025B8151
  // 152,0,0,025B8152025B8152
  // 153,0,0,025B8153025B8153
  // 154,0,0,025B8154025B8154
  // 155,0,0,025B8155025B8155
  debugLoop("called, MCP2515number = % d", MCP2515number);
  char msgString[128];                                                // Array to store serial string
  if ((rxId & 0x80000000) == 0x80000000)                              // Determine if ID is standard (11 bits) or extended (29 bits)
    sprintf(msgString, "%.8lX,0,0,", (rxId & 0x1FFFFFFF));
  else
    sprintf(msgString, "%.3lX,0,0,", rxId);


  Serial.print(msgString);

  if ((rxId & 0x40000000) == 0x40000000) {                            // Determine if message is a remote request frame.
    sprintf(msgString, " REMOTE REQUEST FRAME");
    Serial.print(msgString);
  }
  else {
    for (byte i = 0; i < len; i++) {
      sprintf(msgString, "%.2X", rxBuf[i]);
      Serial.print(msgString);
    }
  }
  Serial.print("\n");
}


void outputFormatSavvyCAN(ulong rxId, uint8_t len, uint8_t rxBuf[], uint8_t MCP2515number) {
  // SavvyCan Format (.GVRET file format) - Validated as working Sep 2024
  // Time Stamp,ID,Extended,Bus,LEN,D1,D2,D3,D4,D5,D6,D7,D8
  // 076733911,59B,false,1,8,00,31,7D,9B,00,31,7D,9B
  debugLoop("called, MCP2515number = % d", MCP2515number);

  if (!headerFirstRun) {
    Serial.printf("Time Stamp,ID,Extended,Bus,LEN,D1,D2,D3,D4,D5,D6,D7,D8\n");
    headerFirstRun = true;
  }
  if ((rxId & 0x80000000) == 0x80000000) {         // Determine if ID is standard (11 bits) or extended (29 bits)
    Serial.printf("%.9lu,%.8lX,true,%1d,%1d", micros() - upTimer, (rxId & 0x1FFFFFFF), MCP2515number, len);
  }
  else {
    Serial.printf("%.9lu,%.3lX,false,%1d,%1d", micros() - upTimer, rxId, MCP2515number, len);
  }
  
  if ((rxId & 0x40000000) == 0x40000000) {    // Determine if message is a remote request frame.
    Serial.printf(" REMOTE REQUEST FRAME");
  }
  else {
    for (byte i = 0; i < len; i++) {
      Serial.printf(",%.2X", rxBuf[i]);
    }
  }
  Serial.printf("\n");
}


// Writes a CAN Frame to the SD Card Device (OpenLager) in SavvyCAN compatible format
void OutputSDCardSavvyCAN(ulong rxId, uint8_t len, uint8_t rxBuf[], uint8_t MCP2515number) {
  // Time Stamp,ID,Extended,Bus,LEN,D1,D2,D3,D4,D5,D6,D7,D8
  // 076733911,59B,false,1,8,00,31,7D,9B,00,31,7D,9B
  debugLoop("called, MCP2515number = % d", MCP2515number);

  if (!sdCardFirstRun) {
    SD_Port.printf("Time Stamp,ID,Extended,Bus,LEN,D1,D2,D3,D4,D5,D6,D7,D8\n");
    sdCardFirstRun = true;
  }

  if ((rxId & 0x80000000) == 0x80000000)          // Determine if ID is standard (11 bits) or extended (29 bits)
    SD_Port.printf("%.9lu,%.8lX,true,%1d,%1d", micros() - upTimer, (rxId & 0x1FFFFFFF), MCP2515number, len);
  else
    SD_Port.printf("%.9lu,%.3lX,false,%1d,%1d", micros() - upTimer, rxId, MCP2515number, len);

  if ((rxId & 0x40000000) == 0x40000000) {    // Determine if message is a remote request frame.
    SD_Port.printf(" REMOTE REQUEST FRAME");
  }
  else {
    for (byte i = 0; i < len; i++) {
      SD_Port.printf(",%.2X", rxBuf[i]);
    }
    SD_Port.printf("\n");
  }

  if(displayPointInTime == BTN_YES) DisplayPointInTime(frame.can_id, frame.can_dlc, frame.data, MCP2515number, 140);
}


void DisplayPointInTime(ulong rxId, uint8_t len, uint8_t rxBuf[], uint8_t MCP2515number, uint8_t yOffset) {
  debugLoop("called, MCP2515number = % d", MCP2515number);

  // control variables for date and time
  static uint8_t  seconds = 0;
  static uint8_t  MSSeconds = 0;
  static uint8_t  MS0x490D3 = 0;
  static uint8_t  HSSeconds = 0;
  static uint8_t  HS0x3D3D1 = 0;

  // detect MS date & time signal has changed
  if (MCP2515number == 2 && rxId == 0x04D2 && seconds != rxBuf[7]) {
    debugLoop("Date & Time\n");
    seconds = rxBuf[7];         // Store new seconds
    char dateTime[19];
    sprintf(dateTime, "%02d/%02d/%02d  %02d:%02d:%02d", rxBuf[4], rxBuf[3], rxBuf[2], rxBuf[5], rxBuf[6], rxBuf[7]);
    //String dateTime = String(rxBuf[4]) + "/" + String(rxBuf[3]) + "/" + String(rxBuf[2]) + "  " + String(rxBuf[5]) + ":" + String(rxBuf[6]) + ":" + String(rxBuf[7]);
    TFT_Rectangle_ILI9341.fillRect(145, yOffset, 155, 20, TFT_LANDROVERGREEN);
    TFT_Rectangle_ILI9341.setTextColor(TFT_GREEN);
    TFT_Rectangle_ILI9341.drawString(dateTime, 220, yOffset + 10);
  }


  // detect MS car age signal has changed
  else if (MCP2515number == 2 && rxId == 0x0405 && rxBuf[0] == 0x01 && MSSeconds != rxBuf[5]) {
    debugLoop("MS Car Age\n");
    MSSeconds = rxBuf[5];         // Store new seconds
    uint32_t canValue = ((rxBuf[2] << 24) | (rxBuf[3] << 16) | (rxBuf[4] << 8) | rxBuf[5]) * 0.1;
    TFT_Rectangle_ILI9341.fillRect(145, yOffset + 20, 105, 20, TFT_LANDROVERGREEN);
    TFT_Rectangle_ILI9341.setTextColor(TFT_GREEN);
    TFT_Rectangle_ILI9341.drawString(String(canValue), 195, yOffset + 30);
  }

  // detect MS 0x490 D3 signal has changed
  else if (MCP2515number == 2 && rxId == 0x0490 && MS0x490D3 != rxBuf[3]) {
    debugLoop("MS 0x490 D3\n");
    MS0x490D3 = rxBuf[3];         // Store new value
    TFT_Rectangle_ILI9341.fillRect(145, yOffset + 40, 40, 20, TFT_LANDROVERGREEN);
    TFT_Rectangle_ILI9341.setTextColor(TFT_GREEN);
    TFT_Rectangle_ILI9341.drawString(String(MS0x490D3), 160, yOffset + 50);
  }


  // detect HS car age signal has changed
  else if (MCP2515number == 1 && rxId == 0x0405 && rxBuf[0] == 0x01 && HSSeconds != rxBuf[5]) {
    debugLoop("HS Car Age\n");
    HSSeconds = rxBuf[5];         // Store new seconds
    uint32_t canValue = ((rxBuf[2] << 24) | (rxBuf[3] << 16) | (rxBuf[4] << 8) | rxBuf[5]) * 0.1;
    TFT_Rectangle_ILI9341.fillRect(145, yOffset + 60, 105, 20, TFT_LANDROVERGREEN);
    TFT_Rectangle_ILI9341.setTextColor(TFT_GREEN);
    TFT_Rectangle_ILI9341.drawString(String(canValue), 195, yOffset + 70);
  }

  // detect HS 0x369 D1 signal has changed
  else if (MCP2515number == 1 && rxId == 0x03D3 && HS0x3D3D1 != rxBuf[1]) {
    debugLoop("HS 0x315 D4\n");
    HS0x3D3D1 = rxBuf[1];         // Store new value
    TFT_Rectangle_ILI9341.fillRect(145, yOffset + 80, 40, 20, TFT_LANDROVERGREEN);
    TFT_Rectangle_ILI9341.setTextColor(TFT_GREEN);
    TFT_Rectangle_ILI9341.drawString(String(HS0x3D3D1), 160, yOffset + 90);
  }

}


void DisplayTestingData(ulong rxId, uint8_t len, uint8_t rxBuf[], uint8_t MCP2515number) {
  debugLoop("called, MCP2515number = % d", MCP2515number);

  static unsigned long timerMillis = millis();
  static byte sendMessageCounter = 0;

  // Control the Requests for Data sent on the CAN Bus to the vehicle
  // This will only send one "Request for Data" every x milliseconds
  // sendMessageCounter controls which message will be sent, upto 10 messages
  // On Land Rover Freelander 2 is seems ok to keep requesting data even though you may not always get a response, thus there are no checks to ensure responses
  // On Land Rover Freelander 2 all requests must be sent on the High Speed CAN Bus (500kbps)
  /*
  **************************************************************
  *                         CAUTION                            *
  * Sending messages to a car can have disastrous consequences *
  *     Only send the correct messages for your car !!         *
  *                 YOU HAVE BEEN WARNED !!                    *
  **************************************************************
  */
  if (millis() - timerMillis > 100) {
    timerMillis = millis();
    if (sendMessageCounter++ > 10) sendMessageCounter = 1;

    // sendMessageCounter will loop from 1 to 10 to match the output line numbers on the displayed data
    // although sometimes you may want to request the same data several times per second in which case
    // you will need to use an unused display line, for example send on 1, 4 and 7

    // Request BCM 292 Analogue input 11
    if (sendMessageCounter == 1) {
      debugSpecial("%ld Send 0x0726 Battery Voltage Request\n", millis());
      frame.can_id = 0x726;
      frame.can_dlc = 0x08;
      frame.data[0] = 0x03;
      frame.data[1] = 0x22;
      frame.data[2] = 0xD9;
      frame.data[3] = 0x11;
      frame.data[4] = 0x00;
      frame.data[5] = 0x00;
      frame.data[6] = 0x00;
      frame.data[7] = 0x00;
      // Never use the direct call mcp2515_1.sendMassage(&frame)!!
#if ALLOW_SENDING_DATA_TO_CAN_BUS == 1
      CANBusSendCANData(mcp2515_1);
#endif
    }


//    // Unused
//    else if (sendMessageCounter == 2) {
//      debugSpecial("%ld Unused: 2\n", millis());
//      frame.can_id = 0x000;
//      frame.can_dlc = 0x00;
//      frame.data[0] = 0x00;
//      frame.data[1] = 0x00;
//      frame.data[2] = 0x00;
//      frame.data[3] = 0x00;
//      frame.data[4] = 0x00;
//      frame.data[5] = 0x00;
//      frame.data[6] = 0x00;
//      frame.data[7] = 0x00;
//      // Never use the direct call mcp2515_1.sendMassage(&frame)!!
//#if ALLOW_SENDING_DATA_TO_CAN_BUS == 1
//      CANBusSendCANData(mcp2515_1);
//#endif
//    }


    // Request BCM 141 Vehicle battery state of charge
    else if (sendMessageCounter == 3) {
      debugSpecial("%ld Send 0x0726 Battery SoC Request\n", millis());
      frame.can_id = 0x726;
      frame.can_dlc = 0x08;
      frame.data[0] = 0x03;
      frame.data[1] = 0x22;
      frame.data[2] = 0x40;
      frame.data[3] = 0x28;
      frame.data[4] = 0x00;
      frame.data[5] = 0x00;
      frame.data[6] = 0x00;
      frame.data[7] = 0x00;
      // Never use the direct call mcp2515_1.sendMassage(&frame)!!
#if ALLOW_SENDING_DATA_TO_CAN_BUS == 1
      CANBusSendCANData(mcp2515_1);
#endif
    }


//    // BCM 218 Power status time data capture - PID 4028: Vehicle battery state of charge
//    else if (sendMessageCounter == 4) {
//      debugSpecial("%ld Send 0x0726 PID Battery SoC Request\n", millis());
//      // This seems to need two calls
//      frame.can_id = 0x726;
//      frame.can_dlc = 0x08;
//      frame.data[0] = 0x30;
//      frame.data[1] = 0x00;
//      frame.data[2] = 0x00;
//      frame.data[3] = 0x00;
//      frame.data[4] = 0x00;
//      frame.data[5] = 0x00;
//      frame.data[6] = 0x00;
//      frame.data[7] = 0x00;
//      // Never use the direct call mcp2515_1.sendMassage(&frame)!!
//#if ALLOW_SENDING_DATA_TO_CAN_BUS == 1
//      CANBusSendCANData(mcp2515_1);
//#endif
//
//      frame.can_id = 0x726;
//      frame.can_dlc = 0x08;
//      frame.data[0] = 0x03;
//      frame.data[1] = 0x22;
//      frame.data[2] = 0x41;
//      frame.data[3] = 0xC3;
//      frame.data[4] = 0x00;
//      frame.data[5] = 0x00;
//      frame.data[6] = 0x00;
//      frame.data[7] = 0x00;
//      // Never use the direct call mcp2515_1.sendMassage(&frame)!!
//#if ALLOW_SENDING_DATA_TO_CAN_BUS == 1
//      CANBusSendCANData(mcp2515_1);
//#endif
//    }


//    // Unused
//    else if (sendMessageCounter == 5) {
//      debugSpecial("%ld Unused: 5\n", millis());
//      frame.can_id = 0x000;
//      frame.can_dlc = 0x00;
//      frame.data[0] = 0x00;
//      frame.data[1] = 0x00;
//      frame.data[2] = 0x00;
//      frame.data[3] = 0x00;
//      frame.data[4] = 0x00;
//      frame.data[5] = 0x00;
//      frame.data[6] = 0x00;
//      frame.data[7] = 0x00;
//      // Never use the direct call mcp2515_1.sendMassage(&frame)!!
//#if ALLOW_SENDING_DATA_TO_CAN_BUS == 1
//      CANBusSendCANData(mcp2515_1);
//#endif
//    }
//
//
//    // Unused
//    else if (sendMessageCounter == 6) {
//      debugSpecial("%ld Unused: 6\n", millis());
//      frame.can_id = 0x000;
//      frame.can_dlc = 0x00;
//      frame.data[0] = 0x00;
//      frame.data[1] = 0x00;
//      frame.data[2] = 0x00;
//      frame.data[3] = 0x00;
//      frame.data[4] = 0x00;
//      frame.data[5] = 0x00;
//      frame.data[6] = 0x00;
//      frame.data[7] = 0x00;
//      // Never use the direct call mcp2515_1.sendMassage(&frame)!!
//#if ALLOW_SENDING_DATA_TO_CAN_BUS == 1
//      CANBusSendCANData(mcp2515_1);
//#endif
//    }
//
//
//    // Unused
//    else if (sendMessageCounter == 7) {
//      debugSpecial("%ld Unused: 7\n", millis());
//      frame.can_id = 0x000;
//      frame.can_dlc = 0x00;
//      frame.data[0] = 0x00;
//      frame.data[1] = 0x00;
//      frame.data[2] = 0x00;
//      frame.data[3] = 0x00;
//      frame.data[4] = 0x00;
//      frame.data[5] = 0x00;
//      frame.data[6] = 0x00;
//      frame.data[7] = 0x00;
//      // Never use the direct call mcp2515_1.sendMassage(&frame)!!
//#if ALLOW_SENDING_DATA_TO_CAN_BUS == 1
//      CANBusSendCANData(mcp2515_1);
//#endif
//    }


    // RDCM 26 Propulsion shaft torque 0.000 lbf ft
    else if (sendMessageCounter == 4 || sendMessageCounter == 8) {
      debugSpecial("%ld Send 0x0795 Propulsion torque  lbf ft\n", millis());
      frame.can_id = 0x795;
      frame.can_dlc = 0x08;
      frame.data[0] = 0x03;
      frame.data[1] = 0x22;
      frame.data[2] = 0xD9;
      frame.data[3] = 0x62;
      frame.data[4] = 0x00;
      frame.data[5] = 0x00;
      frame.data[6] = 0x00;
      frame.data[7] = 0x00;
      // Never use the direct call mcp2515_1.sendMassage(&frame)!!
#if ALLOW_SENDING_DATA_TO_CAN_BUS == 1
      CANBusSendCANData(mcp2515_1);
#endif
    }


    // RDCM 26 All wheel drive propulsion shaft torque 0.000 lbf ft
    else if (sendMessageCounter == 5 || sendMessageCounter == 9) {
      debugSpecial("%ld Send 0x0795 All wheel drive propulsion shaft torque\n", millis());
      frame.can_id = 0x795;
      frame.can_dlc = 0x08;
      frame.data[0] = 0x03;
      frame.data[1] = 0x22;
      frame.data[2] = 0xD9;
      frame.data[3] = 0x30;
      frame.data[4] = 0x00;
      frame.data[5] = 0x00;
      frame.data[6] = 0x00;
      frame.data[7] = 0x00;
      // Never use the direct call mcp2515_1.sendMassage(&frame)!!
#if ALLOW_SENDING_DATA_TO_CAN_BUS == 1
      CANBusSendCANData(mcp2515_1);
#endif
    }


    // RDCM 23 Control module internal temperature
    else if (sendMessageCounter == 10) {
      debugSpecial("%ld Send 0x0795 Control module internal temperature\n", millis());
      frame.can_id = 0x795;
      frame.can_dlc = 0x08;
      frame.data[0] = 0x03;
      frame.data[1] = 0x22;
      frame.data[2] = 0xD1;
      frame.data[3] = 0x16;
      frame.data[4] = 0x00;
      frame.data[5] = 0x00;
      frame.data[6] = 0x00;
      frame.data[7] = 0x00;
      // Never use the direct call mcp2515_1.sendMassage(&frame)!!
#if ALLOW_SENDING_DATA_TO_CAN_BUS == 1
      CANBusSendCANData(mcp2515_1);
#endif
    }

    // Add more data requests here, max sendMessageCounter == 9

  }

  // Line 1
  // BCM 292 Analogue input 11
  if (MCP2515number == 1 && rxId == 0x072E && rxBuf[0] == 0x07 && rxBuf[1] == 0x62 && rxBuf[2] == 0xD9 && rxBuf[3] == 0x11) {
    byte testDataLineNumber = 1;
    float newValue = ((rxBuf[4] << 8) | rxBuf[5]) * 0.01;
    static float oldValue;
    debugSpecial("Battery Voltage = %f\n", newValue);
    if (oldValue != newValue) {
      oldValue = newValue;
      TFT_Rectangle_ILI9341.fillRect(145, 20 * testDataLineNumber - 10, 110, 20, TFT_LANDROVERGREEN);
      TFT_Rectangle_ILI9341.setTextColor(TFT_GREEN);
      TFT_Rectangle_ILI9341.drawString(String(newValue) + "V", 200, 20 * testDataLineNumber);
    }
  }

  // Line 2
  // Unused
  else if (MCP2515number == 1 && rxId == 0x0000) {
    byte testDataLineNumber = 2;
    byte newValue = 0; //rxBuf[4];
    static byte oldValue;
    debugSpecial("Test Line 2\n", newValue);
    if (oldValue != newValue) {
      oldValue = newValue;
      TFT_Rectangle_ILI9341.fillRect(145, 20 * testDataLineNumber - 10, 110, 20, TFT_LANDROVERGREEN);
      TFT_Rectangle_ILI9341.setTextColor(TFT_GREEN);
      TFT_Rectangle_ILI9341.drawString(String(newValue) + "", 200, 20 * testDataLineNumber);
    }
  }

  // Line 3
  // BCM 141 Vehicle battery state of charge
  else if (MCP2515number == 1 && rxId == 0x072E && rxBuf[0] == 0x04 && rxBuf[1] == 0x62 && rxBuf[2] == 0x40 && rxBuf[3] == 0x28) {
    byte testDataLineNumber = 3;
    byte newValue = rxBuf[4];
    static byte oldValue;
    debugSpecial("Battery PID SoC = %d\n", newValue);
    if (oldValue != newValue) {
      oldValue = newValue;
      TFT_Rectangle_ILI9341.fillRect(145, 20 * testDataLineNumber - 10, 110, 20, TFT_LANDROVERGREEN);
      TFT_Rectangle_ILI9341.setTextColor(TFT_GREEN);
      TFT_Rectangle_ILI9341.drawString(String(newValue) + "%", 200, 20 * testDataLineNumber);
    }
  }

  // Line 4
  // BCM 218 Power status time data capture - PID 4028: Vehicle battery state of charge
  else if (MCP2515number == 1 && rxId == 0x072E && rxBuf[0] == 0x22 && rxBuf[1] == 0x00 && rxBuf[2] == 0x10 && rxBuf[3] == 0x01) {
    byte testDataLineNumber = 4;
    byte newValue = rxBuf[5];
    static byte oldValue;
    debugSpecial("Battery PID SoC = %d\n", newValue);
    if (oldValue != newValue) {
      oldValue = newValue;
      TFT_Rectangle_ILI9341.fillRect(145, 20 * testDataLineNumber - 10, 110, 20, TFT_LANDROVERGREEN);
      TFT_Rectangle_ILI9341.setTextColor(TFT_GREEN);
      TFT_Rectangle_ILI9341.drawString(String(newValue) + "%", 200, 20 * testDataLineNumber);
    }
  }

  // Line 5
  // Unused
  else if (MCP2515number == 2 && rxId == 0x0000) {
    byte testDataLineNumber = 5;
    int64_t newValue = 0; // example (rxBuf[3] << 8) | rxBuf[4];
    static int64_t oldValue;
    debugSpecial("Test Line 5\n");
    if (oldValue != newValue) {
      oldValue = newValue;
      TFT_Rectangle_ILI9341.fillRect(145, 20 * testDataLineNumber - 10, 110, 20, TFT_LANDROVERGREEN);
      TFT_Rectangle_ILI9341.setTextColor(TFT_GREEN);
      TFT_Rectangle_ILI9341.drawString(String(newValue), 200, 20 * testDataLineNumber);
    }
  }

  // Line 6
  // Unused
  else if (MCP2515number == 2 && rxId == 0x0000) {
    byte testDataLineNumber = 6;
    int64_t newValue = 0;
    static int64_t oldValue;
    debugSpecial("Test Line 6\n");
    if (oldValue != newValue) {
      oldValue = newValue;
      TFT_Rectangle_ILI9341.fillRect(145, 20 * testDataLineNumber - 10, 110, 20, TFT_LANDROVERGREEN);
      TFT_Rectangle_ILI9341.setTextColor(TFT_GREEN);
      TFT_Rectangle_ILI9341.drawString(String(newValue), 200, 20 * testDataLineNumber);
    }
  }

  // Line 7
  // Unused
  else if (MCP2515number == 1 && rxId == 0x0000) {
    byte testDataLineNumber = 7;
    int64_t newValue = 0;
    static int64_t oldValue;
    debugSpecial("Test Line 7\n");
    if (oldValue != newValue) {
      oldValue = newValue;
      TFT_Rectangle_ILI9341.fillRect(145, 20 * testDataLineNumber - 10, 110, 20, TFT_LANDROVERGREEN);
      TFT_Rectangle_ILI9341.setTextColor(TFT_GREEN);
      TFT_Rectangle_ILI9341.drawString(String(newValue), 200, 20 * testDataLineNumber);
    }
  }

  // Line 8
  // RDCM 29 Propulsion torque  lbf ft
  else if (MCP2515number == 1 && rxId == 0x079D && rxBuf[0] == 0x05 && rxBuf[1] == 0x62 && rxBuf[2] == 0xD9 && rxBuf[3] == 0x62) {
  byte testDataLineNumber = 8;
  int64_t newValue = (rxBuf[4] << 8) | rxBuf[5];
    static int64_t oldValue;
    debugSpecial("Front Torque = %d\n", newValue);
    if (oldValue != newValue) {
      oldValue = newValue;
      TFT_Rectangle_ILI9341.fillRect(145, 20 * testDataLineNumber - 10, 110, 20, TFT_LANDROVERGREEN);
      TFT_Rectangle_ILI9341.setTextColor(TFT_GREEN);
      TFT_Rectangle_ILI9341.drawString(String(newValue) + " lbf ft", 200, 20 * testDataLineNumber);
    }
  }


  // Line 9
  // RDCM 26 All wheel drive propulsion shaft torque 0.000 lbf ft
  else if (MCP2515number == 1 && rxId == 0x079D && rxBuf[0] == 0x05 && rxBuf[1] == 0x62 && rxBuf[2] == 0xD9 && rxBuf[3] == 0x30) {
  byte testDataLineNumber = 9;
  byte newValue = (rxBuf[4] << 8) | rxBuf[5];
    static byte oldValue;
    debugSpecial("Rear Torque = %d\n", newValue);
    if (oldValue != newValue) {
      oldValue = newValue;
      TFT_Rectangle_ILI9341.fillRect(145, 20 * testDataLineNumber - 10, 110, 20, TFT_LANDROVERGREEN);
      TFT_Rectangle_ILI9341.setTextColor(TFT_GREEN);
      TFT_Rectangle_ILI9341.drawString(String(newValue) + " lbf ft", 200, 20 * testDataLineNumber);
    }
  }


  // Line 10
  // RDCM 23 Control module internal temperature C
  else if (MCP2515number == 1 && rxId == 0x079D && rxBuf[0] == 0x04 && rxBuf[1] == 0x62 && rxBuf[2] == 0xD1 && rxBuf[3] == 0x16) {
  byte testDataLineNumber = 10;
  int8_t newValue = rxBuf[4];
    static int64_t oldValue;
    debugSpecial("Control module internal temperature = %d\n", newValue);
    if (oldValue != newValue) {
      oldValue = newValue;
      TFT_Rectangle_ILI9341.fillRect(145, 20 * testDataLineNumber - 10, 110, 20, TFT_LANDROVERGREEN);
      TFT_Rectangle_ILI9341.setTextColor(TFT_GREEN);
      TFT_Rectangle_ILI9341.drawString(String(newValue) + "C", 200, 20 * testDataLineNumber);
    }
  }


}
