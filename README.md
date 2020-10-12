# arduino library for BK8000L 

This directory contain all files needed to support A2DP bluetooth module based on Beken BK8000L chip.

BK8000L (F-6188 module at least) have AT command control support, supported commands are described <a href="https://github.com/tomaskovacik/kicad-library/tree/master/library/datasheet/F-6188_BK8000L">here</a>

# how to connect BK8000L

## directly to computer

this is for testing if your module has serial interface enabled:

BK8000L|computer(USB2serial for example)
-------|-------
   RX  |  TX
   TX  |  RX
  GND  |  GND
 VBAT  |  via diode to 5V

## arduino using software serial (UNO, nano..)

Uncoment "#define USE_SW_SERIAL" in BK8000L.h to enable software serial support.

BK8000L|computer(USB2serial for example)
-------|-------
   RX  |  7 (1st parameter of SoftwareSerial in example)
   TX  |  6 (2nd parameter of SoftwareSerial in example)
 RESET |  5 (defined as resetBTpin in example)
  GND  |  GND
 VBAT  |  via diode to 5V


## arduino using harware serial (mega ..)

BK8000L|computer(USB2serial for example)
-------|-------
   RX  |  TX1 (if using Serial1)
   TX  |  RX1 (if using Serial1)
 RESET |  5 (defined as resetBTpin in example)
  GND  |  GND
 VBAT  |  via diode to 5V


# how to use it

<a href="https://www.arduino.cc/en/Guide/Libraries">Information about using libraries on arduino site</a>

Copy content of this repository directory or just this two files: <a href="https://github.com/tomaskovacik/BK8000L/blob/master/BK8000L.cpp">BK8000L.cpp</a>, <a href="https://github.com/tomaskovacik/BK8000L/blob/master/BK8000L.h">BK8000L.h</a> to ~/Arduino/libraries/BK8000L/ directory (if did not exist, create one). Open new project in arduino and use this code, or check code in examples directory <a href="https://github.com/tomaskovacik/BK8000L/blob/master/examples/BK8000L/BK8000L.ino">BK8000L.ino</a>:

```c
#include "BK8000L.h"
#include <SoftwareSerial.h> //if using SW, with HW no need for this

#define resetBTPin 5
 
SoftwareSerial swSerial(7, 6); //rxPin, txPin

BK8000L BT(&swSerial, resetBTPin); //in case of HW serial use for example: (&Serial1)

void(){
  BT.begin(); //or BT.begin(9600); for specific baudrate
}

void loop(){
//should be call periodically, to get notifications from module, for example if someone calling...
BT.getNextEventFromBT();
}
```

for more examples look ate examples/BK8000L/BK8000L.ino

# supported functions
```c
begin(uint32_t baudrate); //parameter baudrate is communication speed between MCU and arduino, default 9600

sendData(String cmd); //construct string of AT+cmd and send it to serial port 

sendAPTData(String cmd); //construct string of APT+cmd and send it to serial port 

getNextEventFromBT(); //parse data send from module and send internal variales, call this periodicaly, to parse data received from module ASAP

PairingInit(); //initialize pairing mode

PairingExit(); //exit pairing mode

ConnectLastDevice(); //connect to last device 

Disconnect(); //disconnect from currently connected device

callAnsware(); //answare currently receiving call, see CallerID variable to see who is calling

callReject(); //reject currently receiving call,  see CallerID variable to see who is calling

callHangUp(); //hang up current call, see CallerID variable to see who you are calling with

callRedial(); //call last dialed number, see CallerID variable to see who you dialing

volumeUp(); //set volume up

volumeDown(); //set volume down

languageSwitch(); //did not understand what is this doing based on provided info in datasheet (sending AT+CM)

channelSwitch(); //did not understand what is this doing based on provided info in datasheet (sending AT+CO)

shutdownBT(); //shutdown BT module

switchInput(); //switch to AUX input 

openPhoneVoice(); //did not understand what is this doing based on provided info in datasheet (sending AT+CV)

memoryClear(); //did not understand what is this doing based on provided info in datasheet (sending AT+CZ)

languageSetNumber(uint8_t number); //did not understand what is this doing based on provided info in datasheet (sending AT+CM+number)

musicTogglePlayPause(); //toggle play/pause music on mobile phone

musicStop(); //stop playing music

musicNextTrack(); //switch to next track

musicPreviousTrack(); //switch to previous track

musicFastForward(); //fast forward current track

musicRewind(); //rewind current track

getName(); //get bluetooth module name

getConnectionStatus(); //get BT connection status 

getPinCode(); //get pin

getAddress(); //get HW address of module

getSoftwareVersion(); //get SW version of module (this one did not work for me)

MusicGetStatus(); //get playback status 

getHFPstatus(); //get BT connection status

resetModule(); //reset module, for example to start it again
```

# Handling responce from module

As far as getNextEventFromBT() is called periodically, the library will update state variables based on the response from module. Function getNextEventFromBT() is called at the end of all functions mentioned above to handle response sent from the bluetooth module asap.

Library has its own variables, which are set/updated when data are received from module:
```c
String BT_ADDR; // updated after getAddress() is called

String BT_NAME; // updated after getName() is called

String BT_PIN; // updated after getPinCode() is called

String CallerID; // updated automatically or by calling getHFPstatus()

uint8_t BTState; // enum, updated automatically or by calling getHFPstatus() or getConnectionStatus()

uint8_t CallState; // enum, updated automatically or by calling callRedial() or getHFPstatus()

uint8_t MusicState; // enum, updated automatically  or by calling getMusicStatus()

uint8_t PowerState; // enum, updated automaticaly or by calling getHFPstatus(),getMusicStatus() or getConnectionStatus()
```
If you init BK8000L library as in example code this variables can be accessed directly from main sketch with prefix BT, for example:
```c
Serial.print(BT.BT_NAME);
```

BTState, CallState, MusicState and PowerState have these states, these are accessible from main sketch with prefix like other variable:
```c
BT.Playing; // responce from module "MA"

BT.Idle; // responce from module "MB"

BT.IncomingCall; // responce from module "IR- or M2"

BT.OutgoingCall; // responce from module "PR- or M3"

BT.CallInProgress; // responce from module "M4"

BT.Connected; // responce from module "M1"

BT.Disconnected; // responce from module "M0"

BT.On; // responce from module "ON"

BT.Off; // set if shutdownBT() is called

BT.Pairing; // set if PairingInit() is called
```

these can be used to triger actions in main sketch based on change of module state see <a href="https://github.com/tomaskovacik/BK8000L/blob/master/examples/BK8000L/BK8000L.ino">BK8000L.ino</a> in examples directory.





