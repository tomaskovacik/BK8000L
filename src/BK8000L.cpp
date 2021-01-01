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
#else
BK8000L::BK8000L(HardwareSerial *ser, uint8_t resetPin)
#endif
{
  btSerial = ser;
  _reset=resetPin;
}

/*
   Destructor
*/
BK8000L::~BK8000L() {
 btSerial->end();
}


void BK8000L::begin(uint32_t baudrate) {
  btSerial->begin(baudrate);
  pinMode(_reset,OUTPUT);
  BK8000L::resetHigh();
}

void BK8000L::resetLow(){
 digitalWrite(_reset,LOW);
}

void BK8000L::resetHigh(){
 digitalWrite(_reset,HIGH);
}

/*
   debug output
*/
#if defined DEBUG
void BK8000L::DBG(String text) {
  Serial.print(text);;
}
#endif


void BK8000L::resetModule(){
#if defined DEBUG
 DBG(F("Reseting module"));
#endif
 resetLow();
 delay(100);
 resetHigh();
}

uint8_t BK8000L::decodeReceivedString(String receivedString) {
#if defined DEBUG
  DBG(receivedString);
  DBG(F("\n"));
#endif
  switch (receivedString[0]) {
    case 'A':
      {
      PowerState=On;
        if (receivedString[1] == 'D' && receivedString[2] == ':') {
            BT_ADDR = receivedString.substring(5);
#if defined DEBUG
            DBG(F("BT ADDRESS: ")); DBG(BT_ADDR);
#endif
        }
        if (receivedString[1] == 'P' && receivedString[2] == 'R' && receivedString[3] == '+'){
#if defined DEBUG
            DBG(F("SPP data received: ")); DBG(receivedString.substring(5));
#endif
	    receivedSppData=receivedString.substring(4);
	}
#ifdef BK8002
	if (memcmp(&receivedString[0], "AUX_MODE", 8) == 0)
          mode = AUX;
#endif
      }
      break;
#ifdef BK8002
    case 'B':
      {
        if (memcmp(&receivedString[0], "BT_MODE", 7) == 0)
          mode = BT;
      }
      break;
#endif
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
#ifdef BK8002
    case 'F':
      {
        if (receivedString[1] == 'M') {
          if (receivedString[2] != '_' ) //song number
            CurrentFrequency = receivedString.substring(2);//.toInt();//FM freq
	  if (memcmp(&receivedString[0], "FM_MODE", 7) == 0)
            mode = FM; //FM_MODE
        }
      }
      break;
#endif
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
#ifdef BK8002
          case 'P':
            if (receivedString[2] == '3') {
              if (receivedString[3] != '_' ) //song number
                songNumber = receivedString.substring(3).toInt();//this should give as just song number, if it can handle endline at the end of string
              if (memcmp(&receivedString[0], "MP3_MODE", 8) == 0)
                mode = SDCARD;
            }
            break;
#endif
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
#ifdef BK8002
        case 'L':
          if ( receivedString[2] == 'A' && receivedString[3] == 'Y' && receivedString[2] == '_' ) {
            if (receivedString[2] == 'A' && receivedString[3] == 'L' && receivedString[2] == 'L')
              playMode = ALL;
            if (receivedString[2] == 'O' && receivedString[3] == 'N' && receivedString[2] == 'E')
              playMode = ONE;
          }
        break;
#endif
      }
      }
    break;
    case 'O': //BT On or received OK
      {
        switch (receivedString[1]) {
        case 'N':
          PowerState = On;
          break;
        case 'K':
	  if (PowerState == ShutdownInProgress) PowerState=Off;
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
#if defined DEBUG
	DBG(F("Calling: ")); DBG(receivedString.substring(4,(receivedString.length() - 2))); DBG(F("\n"));
#endif
	return receivedString.substring(4,(receivedString.length() - 2)); //start at 4 cose: IR-"+123456789" or PR-"+123456789" and one before end to remove " and \0
}

String BK8000L::returnBtModuleName(String receivedString) {
#if defined DEBUG
	DBG(F("Bluetooth module name: ")); DBG(receivedString.substring(4)); DBG(F("\n"));
#endif
	return receivedString.substring(4);
}

uint8_t BK8000L::getNextEventFromBT() {
  char c;
  String receivedString = "";
delay(100);
  while (btSerial -> available() > 0) {
    c = (btSerial -> read());
    if (c == 0xD) {
      if (receivedString == "") { //nothing before enter was received
#if defined DEBUG
        DBG(F("received only empty string\n running again myself...\n"));
#endif
      }
      receivedString = receivedString + c;
      return decodeReceivedString(receivedString);
      break;
    }
    //append received buffer with received character
    receivedString = receivedString + c;  // cose += c did not work ...
  }
  return decodeReceivedString(receivedString);
}

uint8_t BK8000L::checkResponce(void){
  uint16_t timeout=500;//500ms -- datasheet did not stated any timeout for "OK" responce, so I give him 500ms
  while (!getNextEventFromBT() && timeout > 0)
  {
    timeout--;
    delay(1); // wait 1milisecond
  }
  if (!timeout) return false;
  return true;
}

uint8_t BK8000L::sendData(String cmd) {
  BK8000L::getNextEventFromBT();
  String Command = "AT+" + cmd + "\r\n";
#if defined DEBUG
  DBG(F("Sending "));DBG(Command);
#endif
  delay(100);
  btSerial -> print(Command);
  return checkResponce();
}

uint8_t BK8000L::sendAPTData(String cmd) {
  BK8000L::getNextEventFromBT();
  String Command = "APT+" + cmd + "\r\n";
#if defined DEBUG
  DBG(F("Sending APT "));DBG(Command);
#endif
  delay(100);
  btSerial -> print(Command);
  return checkResponce();
}

uint8_t BK8000L::aptLogin(){
  BK8000L::getNextEventFromBT();
  return BK8000L::sendAPTData("SPP8888");
}

uint8_t BK8000L::pairingInit() { //  pairing   AT+CA\r\n
  BTState=Pairing;
  return BK8000L::sendData(BK8000L_PAIRING_INIT);
}

uint8_t BK8000L::pairingExit() {//  Exit pairing  AT+CB\r\n
  return BK8000L::sendData(BK8000L_PAIRING_EXIT);
}

uint8_t BK8000L::connectLastDevice() {//  The last paired device connected  AT+CC\r\n     what this should do? connect to last connected device?
  return BK8000L::sendData(BK8000L_CONNECT_LAST_DEVICE);
}

uint8_t BK8000L::disconnect() {//  disconnect  AT+CD\r\n
  return BK8000L::sendData(BK8000L_DISCONNECT);
}

uint8_t BK8000L::callAnsware() { //  Answer the call   AT+CD\r\n
  return BK8000L::sendData(BK8000L_CALL_ANSWARE);
}

uint8_t BK8000L::callReject() { //  reject a call   AT+CF\r\n
  return BK8000L::sendData(BK8000L_CALL_REJECT);
}

uint8_t BK8000L::callHangUp() { //  Hang up   AT+CG\r\n
  return BK8000L::sendData(BK8000L_CALL_HANGUP);
}

uint8_t BK8000L::callRedial() { //  redial  AT+CH\r\n     last called number?
  return BK8000L::sendData(BK8000L_CALL_REDIAL);
}

uint8_t BK8000L::volumeUp() { //  volume up   AT+CK\r\n
  return BK8000L::sendData(BK8000L_VOLUME_UP);
}

uint8_t BK8000L::volumeDown() { //  volume down   AT+CL\r\n
  return BK8000L::sendData(BK8000L_VOLUME_DOWN);
}

uint8_t BK8000L::languageSwitch() { //  Multi-language switch   AT+CM\r\n
  return BK8000L::sendData(BK8000L_LANGUAGE_SWITCH);
}

uint8_t BK8000L::channelSwitch() { //  Channel switching (invalid)   AT+CO\r\n     to be tested
  return BK8000L::sendData(BK8000L_CHANNEL_SWITCH);
}

uint8_t BK8000L::shutdownBT() { //  Shutdown  AT+CP\r\n
  PowerState = ShutdownInProgress; 
  return BK8000L::sendData(BK8000L_SHUTDOWN);
}

uint8_t BK8000L::switchInput() { //  Enter the test mode   AT+CT\r\n
  return BK8000L::sendData(BK8000L_SWITCH_INPUT);
}

uint8_t BK8000L::openPhoneVoice() { //  Open phone VOICE  AT+CV\r\n
  return BK8000L::sendData(BK8000L_OPEN_PHONE_VOICE);
}

uint8_t BK8000L::memoryClear() { //  Memory clear  AT+CZ\r\n
  return BK8000L::sendData(BK8000L_MEMORY_CLEAR);
}

uint8_t BK8000L::languageSetNumber(uint8_t number) { //  Number:( 0-4 )  Set the number of multi-lingual   AT+CMM4\r\n
  String command = BK8000L_LANGUAGE_SET_NUMBER + (String)number;
  return BK8000L::sendData(command);
}

uint8_t BK8000L::musicTogglePlayPause() { //  Music Play / Pause  AT+MA\r\n
  return BK8000L::sendData(BK8000L_MUSIC_TOGGLE_PLAY_PAUSE);
}

uint8_t BK8000L::musicStop() { //  The music stops   AT+MC\r\n
  return BK8000L::sendData(BK8000L_MUSIC_STOP);
}

uint8_t BK8000L::musicNextTrack() { //  next track  AT+MD\r\n
  return BK8000L::sendData(BK8000L_MUSIC_NEXT_TRACK);
}

uint8_t BK8000L::musicPreviousTrack() { //  previous track  AT+ME\r\n
  return BK8000L::sendData(BK8000L_MUSIC_PREVIOUS_TRACK);
}

uint8_t BK8000L::musicFastForward() { //  fast forward  AT+MF\r\n     test how does this exacly works?
  return BK8000L::sendData(BK8000L_MUSIC_FAST_FORWARD);
}

uint8_t BK8000L::musicRewind() { //  rewind  AT+MH\r\n     test how does this exacly works?
  return BK8000L::sendData(BK8000L_MUSIC_REWIND);
}

uint8_t BK8000L::getName() { //  Query bluetooth name  AT+MN\r\n   NA:BK8000L\r\n  test this
  return BK8000L::sendData(BK8000L_GET_NAME);
}

uint8_t BK8000L::getConnectionStatus() { //  Bluetooth connection status inquiry   AT+MO\rn  connection succeeded:" C1\r\n"no connection:"C0\r\n"
  return BK8000L::sendData(BK8000L_GET_CONNECTION_STATUS);
}

uint8_t BK8000L::getPinCode() {           //  PIN Code query  AT+MP\r\n   PN:0000\r\n
  return BK8000L::sendData(BK8000L_GET_PIN_CODE);
}

uint8_t BK8000L::getAddress() { //  Query bluetooth address   AT+MR\r\n   AD:111111111111\r\n
  return BK8000L::sendData(BK8000L_GET_ADDRESS);
}

uint8_t BK8000L::getSoftwareVersion() { //  Query software version  AT+MQ\r\n   XZX-V1.2\r\n
  return BK8000L::sendData(BK8000L_GET_SOFTWARE_VERSION);
}

uint8_t BK8000L::getMusicStatus() { //  Bluetooth playback status inquiry   AT+MV\r\n   Play: "MB\r\n", time out:"MA\r\n", disconnect:" M0\r\n"
  return BK8000L::sendData(BK8000L_MUSIC_GET_STATUS);
}

uint8_t BK8000L::getHFPStatus() { //Bluetooth inquiry HFP status  AT+MY\r\n   disconnect:"M0\r\n", connection:"M1\r\n", Caller: "M2\r\n", Outgoing: "M3\r\n", calling:"M4\r\n"
  return BK8000L::sendData(BK8000L_GET_HFP_STATUS);
}

#ifdef BK8002
uint8_t BK8000L::setVolume(uint8_t volume){ //BK8002_SET_VOLUME "VOL" //Set the volume level AT+VOLx\r\n x=(0-15)
  String command = BK8002_SET_VOLUME + (String)volume;
  return BK8000L::sendData(command);
}

uint8_t BK8000L::setBluetoothMode(){ //BK8002_BLUETOOTH_MODE "MNBT" //Bluetooth mode AT+MNBT\r\n
  return BK8000L::sendData(BK8002_BLUETOOTH_MODE);
}

uint8_t BK8000L::setSdcardMode(){ //BK8002_SDCARD_MODE "MNMP3" //TF mode AT+MNMP3\r\n
  return BK8000L::sendData(BK8002_SDCARD_MODE);
}

uint8_t BK8000L::setAUXMode(){ //BK8002_AUX_MODE "MNAUX" //AUX mode AT+MNAUX\r\n
  return BK8000L::sendData(BK8002_AUX_MODE);
}

uint8_t BK8000L::setFMMode(){ //BK8002_FM_MODE "MNFM" //FM mode AT+MNFM\r\n
  return BK8000L::sendData(BK8002_FM_MODE);
}

uint8_t BK8000L::setSdcardRepeatAll(){ //BK8002_SDCARD_REPEAT_ALL "MPM0" //Loop all ( TF Mode) AT+MPM0\r\n
  return BK8000L::sendData(BK8002_SDCARD_REPEAT_ALL);
}

uint8_t BK8000L::setSdcardRepeaetOne(){ //BK8002_SDCARD_REPEAT_ONE "MPM1" //Single loop playback ( TF Mode) AT+MPM1\r\n
  return BK8000L::sendData(BK8002_SDCARD_REPEAT_ONE);
}

uint8_t BK8000L::setSdcardPlaSong(uint16_t songNumber){ //BK8002_SDCARD_PLAY_SONG "SMP" //Play selection ( TF Mode) AT+SMPXXXX\r\n xxxx :( 0000-9999 ); (" 0000 "On behalf of the 1 first)
  String command = BK8002_SDCARD_PLAY_SONG + (String)songNumber;
  return BK8000L::sendData(command);
}

uint8_t BK8000L::getFMChannelNumer(){ //BK8002_GET_FM_CHANNEL_NUMBER "MRFM" //Inquire FM Channel number ( FM Mode) AT+MRFM\r\n; responce: FM99.8\r\n
  return BK8000L::sendData(BK8002_GET_FM_CHANNEL_NUMBER);
}

uint8_t BK8000L::getSdcardSongNumber(){ //BK8002_GET_SDCARD_SONG_NUMBER "MRMP3" //Inquire MP3 Song number ( TF Mode); AT+MRMP3\r\n; responce: MP3x\r\n
  return BK8000L::sendData(BK8002_GET_SDCARD_SONG_NUMBER);
}

uint8_t BK8000L::getSdcardNumberOfSongs(){ //BK8002_GET_SDCARD_NUMBER_OF_SONGS "MMMP3" //Inquire MP3 Number of songs ( TF Mode); COM+MMMP3\r\n ; responce: MMPx\r\n (x=number of songs)
  return BK8000L::sendData(BK8002_GET_SDCARD_NUMBER_OF_SONGS);
}

uint8_t BK8000L::getSdcardPlayMode(){ //BK8002_GET_SDCARD_PLAY_MODE "MPMC" //Inquire MP3 Play mode ( TF Mode); AT+MPMC\r\n; responce: repreatall: PLAY_ALL\r\n; repeate one: PLAY_ONE\r\n
  return BK8000L::sendData(BK8002_GET_SDCARD_PLAY_MODE);
}

uint8_t BK8000L::getCurrentVolume(){ //BK8002_GET_CURRENT_VOLUME "MVOL" //Query current volume; AT+MVOL\r\n ; responce: VOLx\r\n ( x : Represents the volume level)
  return BK8000L::sendData(BK8002_GET_CURRENT_VOLUME);
}

uint8_t BK8000L::getCurrentMode(){ //BK8002_GET_CURRENT_MODE "MM" //Query current mode; AT+MM\r\n; responce: Bluetooth: BT_MODE\r\n TF : MP3_MODE\r\n FM : FM_MODE\r\n AUX : AUX_MODE\r\n
  return BK8000L::sendData(BK8002_GET_CURRENT_MODE);
}
#endif
