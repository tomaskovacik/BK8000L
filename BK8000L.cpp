/*
        GNU GPL v3
        (C) Tomas Kovacik [nail at nodomain dot sk]
        https://github.com/tomaskovacik/

	BK8000L module library
*/

#include "BK8000L.h"
#include <Arduino.h>

#if defined(USE_SW_SERIAL)
#include <SoftwareSerial.h>
#endif



#if defined(USE_SW_SERIAL)
#if ARDUINO >= 100
BK8000L::BK8000L(SoftwareSerial *ser, uint8_t resetPin)
#else
BK8000L::BK8000L(NewSoftSerial *ser, uint8_t resetPin)
#endif
#endif
{
#if defined(USE_SW_SERIAL)
  btHwSerial = NULL;
  btSwSerial = ser;
  _reset=resetPin;
}
#endif
BK8000L::BK8000L(HardwareSerial *ser, uint8_t resetPin) {
  btSwSerial = NULL;
  btHwSerial = ser;
  _reset=resetPin;
}

/*
   Destructor
*/
BK8000L::~BK8000L() {
}

void BK8000L::begin(uint32_t baudrate) {
#if defined(USE_SW_SERIAL)
  if (btSwSerial)
    btSwSerial->begin(baudrate);
  else
#endif
    btHwSerial->begin(baudrate);
 
  pinMode(_reset,OUTPUT);
  BK8000L::resetHigh();
}

void BK8000L::resetLow(){
 digitalWrite(_reset,LOW);
}

void BK8000L::resetHigh(){
 digitalWrite(_reset,HIGH);
}

void BK8000L::resetModule(){
 DBG("reseting module");
 resetLow();
 delay(100);
 resetHigh();
}

/*
   debug output
*/
void BK8000L::DBG(String text) {
  if (DEBUG) /*return "DBG: ");*/ Serial.print(text);;
}

uint8_t BK8000L::decodeReceivedString(String receivedString) {
  DBG(receivedString);
  DBG("\n");
  switch (receivedString[0]) {
    case 'A':
      {
      PowerState=On;
        if (receivedString[1] == 'D' && receivedString[2] == ':') {
            BT_ADDR = receivedString.substring(5);
            DBG("BT ADDRESS: " + BT_ADDR);
        }
        if (receivedString[1] == 'P' && receivedString[2] == 'R' && receivedString[2] == '+'){
            DBG("SPP data received: " + receivedString.substring(5));
	} 
      }
      break;
    case 'C':
      {
      PowerState=On;
      switch (receivedString[1]) {
        case '1':
          BTState = Connected;
          break;
        case '0':
          BTState = Disconnected;
          break;
      }
      }
      break;
    case 'E':
      {
      PowerState=On;
      switch (receivedString[1]) {
        case 'R':
          if (receivedString[2] == 'R') return 0;
          break;
      }
      }
      break;
    case 'I':// connection info
      {
      PowerState=On;
      switch (receivedString[1]) {
        case 'I': //BT connected
          BTState = Connected;
          break;
        case 'A': //BT disconected
          BTState = Disconnected;
          break;
        case 'R': //caller
          if (receivedString[2] == '-') CallState = IncomingCall;
          CallerID = returnCallerID(receivedString);
          break;
      }
      }
      break;
    case 'M': //music
      {
      PowerState=On;
      switch (receivedString[1]) {
        case 'B':
          MusicState = Playing;
          break;
        case 'A':
          MusicState = Idle;
          break;
        case '0':
          BTState = Disconnected;
          break;
        case '1':
          BTState = Connected;
          break;
        case '2':
          CallState = IncomingCall;
          break;
        case '3':
          CallState = OutgoingCall;
          break;
        case '4':
          CallState = CallInProgress;
          break;
      }
      }
      break;
    case 'N':
      {
      PowerState=On;
        if (receivedString[1] == 'A' && receivedString[2] == ':') {//name
          BT_NAME = BK8000L::returnBtModuleName(receivedString);
        }
      }
      break;
    case 'P':
      {
      PowerState=On;
      switch (receivedString[1]) {
        case 'R': //outgoing call
          if (receivedString[2] == '-') CallState = OutgoingCall;
          CallerID = returnCallerID(receivedString);
        break;
        case 'N':
          if (receivedString[2] == ':') {
            BT_PIN = receivedString.substring(4);
          }
        break;
      }
      }
    break;
    case 'O': //BT On
      {
        switch (receivedString[1]) {
        case 'N':
          PowerState = On;
          break;
        case 'K':
	  if (PowerState == ShutdownInProgress) PowerState=Off;
          return 1;
          break;
      }
      }
    break;
    case 0xA: //\r
	BK8000L::decodeReceivedString(receivedString.substring(1));
    break;
    case 0x20: //space
	BK8000L::decodeReceivedString(receivedString.substring(1));
    break;
}
return 1;
}

String BK8000L::returnCallerID(String receivedString) {
	DBG("Calling: " + receivedString.substring(4,(receivedString.length() - 2)) + "\n");
	return receivedString.substring(4,(receivedString.length() - 2)); //start at 4 cose: IR-"+123456789" or PR-"+123456789" and one before end to remove " and \0
}

String BK8000L::returnBtModuleName(String receivedString) {
	DBG("Bluetooth module name: " + receivedString.substring(4) + "\n");
	return receivedString.substring(4);
}

uint8_t BK8000L::getNextEventFromBT() {

  char c;
  String receivedString = "";

#if defined(USE_SW_SERIAL)
  while (btSwSerial -> available() > 0) {
    c = (btSwSerial -> read());
    //Serial.write(c);Serial.print(" ");Serial.println(c,HEX);
    if (c == 0xD) {
      if (receivedString == "") { //nothing before enter was received
        //DBG("received only empty string\n running again myself...\n");
        BK8000L::getNextEventFromBT();
      }
      receivedString = receivedString + c;
      decodeReceivedString(receivedString);
      break;
    }
    //append received buffer with received character
    receivedString = receivedString + c;  // cose += c did not work ...
  }

#else
  while (btHwSerial -> available() > 0) {
    c = (btHwSerial -> read());
    //Serial.write(c);Serial.print(" ");Serial.println(c,HEX);
    if (c == 0xD) {
      if (receivedString == "") { //nothing before enter was received
        BK8000L::getNextEventFromBT();
      }
      receivedString = receivedString + c;
      decodeReceivedString(receivedString);
      break;
    }
    //append received buffer with received character
    receivedString = receivedString + c;  // cose += c did not work ...
#endif
}

uint8_t BK8000L::sendData(String cmd) {
  String Command = "AT+" + cmd + "\r\n";
  DBG("sending " + Command);
  delay(100);
#if defined(USE_SW_SERIAL)
  if (btSwSerial)
    btSwSerial -> print(Command);
  else
#endif
    btHwSerial -> print(Command);
}

uint8_t BK8000L::sendAPTData(String cmd) {
  String Command = "APT+" + cmd + "\r\n";
  DBG("sending APT " + Command);
  delay(100);
#if defined(USE_SW_SERIAL)
  if (btSwSerial)
    btSwSerial -> print(Command);
  else
#endif
    btHwSerial -> print(Command);
}


uint8_t BK8000L::pairingInit() { //  pairing   AT+CA\r\n
  BK8000L::sendData(BK8000L_PAIRING_INIT);
  BTState=Pairing;
  BK8000L::getNextEventFromBT();
}

uint8_t BK8000L::pairingExit() {//  Exit pairing  AT+CB\r\n
  BK8000L::sendData(BK8000L_PAIRING_EXIT);
  BK8000L::getNextEventFromBT();
}

uint8_t BK8000L::connectLastDevice() {//  The last paired device connected  AT+CC\r\n     what this should do? connect to last connected device?
  BK8000L::sendData(BK8000L_CONNECT_LAST_DEVICE);
  BK8000L::getNextEventFromBT();
}

uint8_t BK8000L::disconnect() {//  disconnect  AT+CD\r\n
  BK8000L::sendData(BK8000L_DISCONNECT);
  BK8000L::getNextEventFromBT();
}

uint8_t BK8000L::callAnsware() { //  Answer the call   AT+CD\r\n
  BK8000L::sendData(BK8000L_CALL_ANSWARE);
  BK8000L::getNextEventFromBT();
}

uint8_t BK8000L::callReject() { //  reject a call   AT+CF\r\n
  BK8000L::sendData(BK8000L_CALL_REJECT);
  BK8000L::getNextEventFromBT();
}

uint8_t BK8000L::callHangUp() { //  Hang up   AT+CG\r\n
  BK8000L::sendData(BK8000L_CALL_HANGUP);
  BK8000L::getNextEventFromBT();
}

uint8_t BK8000L::callRedial() { //  redial  AT+CH\r\n     last called number?
  BK8000L::sendData(BK8000L_CALL_REDIAL);
  BK8000L::getNextEventFromBT();
}

uint8_t BK8000L::volumeUp() { //  volume up   AT+CK\r\n
  BK8000L::sendData(BK8000L_VOLUME_UP);
  BK8000L::getNextEventFromBT();
}

uint8_t BK8000L::volumeDown() { //  volume down   AT+CL\r\n
  BK8000L::sendData(BK8000L_VOLUME_DOWN);
  BK8000L::getNextEventFromBT();
}

uint8_t BK8000L::languageSwitch() { //  Multi-language switch   AT+CM\r\n
  BK8000L::sendData(BK8000L_LANGUAGE_SWITCH);
  BK8000L::getNextEventFromBT();
}

uint8_t BK8000L::channelSwitch() { //  Channel switching (invalid)   AT+CO\r\n     to be tested
  BK8000L::sendData(BK8000L_CHANNEL_SWITCH);
  BK8000L::getNextEventFromBT();
}

uint8_t BK8000L::shutdownBT() { //  Shutdown  AT+CP\r\n
  BK8000L::sendData(BK8000L_SHUTDOWN);
  PowerState = ShutdownInProgress; 
  BK8000L::getNextEventFromBT();
}

uint8_t BK8000L::switchInput() { //  Enter the test mode   AT+CT\r\n
  BK8000L::sendData(BK8000L_SWITCH_INPUT);
  BK8000L::getNextEventFromBT();
}

uint8_t BK8000L::openPhoneVoice() { //  Open phone VOICE  AT+CV\r\n
  BK8000L::sendData(BK8000L_OPEN_PHONE_VOICE);
  BK8000L::getNextEventFromBT();
}

uint8_t BK8000L::memoryClear() { //  Memory clear  AT+CZ\r\n
  BK8000L::sendData(BK8000L_MEMORY_CLEAR);
  BK8000L::getNextEventFromBT();
}

uint8_t BK8000L::languageSetNumber(uint8_t number) { //  Number:( 0-4 )  Set the number of multi-lingual   AT+CMM4\r\n
  //DBG((String)number);
  String command = BK8000L_LANGUAGE_SET_NUMBER + (String)number;
  BK8000L::sendData(command);
  BK8000L::getNextEventFromBT();
}

uint8_t BK8000L::musicTogglePlayPause() { //  Music Play / Pause  AT+MA\r\n
  BK8000L::sendData(BK8000L_MUSIC_TOGGLE_PLAY_PAUSE);
  BK8000L::getNextEventFromBT();
}

uint8_t BK8000L::musicStop() { //  The music stops   AT+MC\r\n
  BK8000L::sendData(BK8000L_MUSIC_STOP);
  BK8000L::getNextEventFromBT();
}

uint8_t BK8000L::musicNextTrack() { //  next track  AT+MD\r\n
  BK8000L::sendData(BK8000L_MUSIC_NEXT_TRACK);
  BK8000L::getNextEventFromBT();
}

uint8_t BK8000L::musicPreviousTrack() { //  previous track  AT+ME\r\n
  BK8000L::sendData(BK8000L_MUSIC_PREVIOUS_TRACK);
  BK8000L::getNextEventFromBT();
}

uint8_t BK8000L::musicFastForward() { //  fast forward  AT+MF\r\n     test how does this exacly works?
  BK8000L::sendData(BK8000L_MUSIC_FAST_FORWARD);
  BK8000L::getNextEventFromBT();
}

uint8_t BK8000L::musicRewind() { //  rewind  AT+MH\r\n     test how does this exacly works?
  BK8000L::sendData(BK8000L_MUSIC_REWIND);
  BK8000L::getNextEventFromBT();
}

uint8_t BK8000L::getName() { //  Query bluetooth name  AT+MN\r\n   NA:BK8000L\r\n  test this
  BK8000L::sendData(BK8000L_GET_NAME);
  BK8000L::getNextEventFromBT();
}

uint8_t BK8000L::getConnectionStatus() { //  Bluetooth connection status inquiry   AT+MO\rn  connection succeeded:" C1\r\n"no connection:"C0\r\n"
  BK8000L::sendData(BK8000L_GET_CONNECTION_STATUS);
  BK8000L::getNextEventFromBT();
}

uint8_t BK8000L::getPinCode() {           //  PIN Code query  AT+MP\r\n   PN:0000\r\n
  BK8000L::sendData(BK8000L_GET_PIN_CODE);
  BK8000L::getNextEventFromBT();
}

uint8_t BK8000L::getAddress() { //  Query bluetooth address   AT+MR\r\n   AD:111111111111\r\n
  BK8000L::sendData(BK8000L_GET_ADDRESS);
  BK8000L::getNextEventFromBT();
}

uint8_t BK8000L::getSoftwareVersion() { //  Query software version  AT+MQ\r\n   XZX-V1.2\r\n
  BK8000L::sendData(BK8000L_GET_SOFTWARE_VERSION);
  BK8000L::getNextEventFromBT();
}

uint8_t BK8000L::getMusicStatus() { //  Bluetooth playback status inquiry   AT+MV\r\n   Play: "MB\r\n", time out:"MA\r\n", disconnect:" M0\r\n"
  BK8000L::sendData(BK8000L_MUSIC_GET_STATUS);
  BK8000L::getNextEventFromBT();
}

uint8_t BK8000L::getHFPstatus() { //Bluetooth inquiry HFP status  AT+MY\r\n   disconnect:"M0\r\n", connection:"M1\r\n", Caller: "M2\r\n", Outgoing: "M3\r\n", calling:"M4\r\n"
  BK8000L::sendData(BK8000L_GET_HFP_STATUS);
  BK8000L::getNextEventFromBT();
}


