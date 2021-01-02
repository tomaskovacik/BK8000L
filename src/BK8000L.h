/*
	GNU GPL v3
	(C) Tomas Kovacik [nail at nodomain dot sk]
	https://github.com/tomaskovacik/

	BK8000L module library

	supports SW and HW serial

	to enable SW serial set USE_SW_SERIAL to 1
	to enable debug output set DEBUG to 1, this is using HW Serial (hardcoded)

*/

#ifndef BK8000L_h
#define BK8000L_h
#include <Arduino.h>

#define USE_SW_SERIAL
//#define BK8002

//#define DEBUG

//cmds definitions:
#define BK8000L_PAIRING_INIT "CA"            //  pairing   AT+CA\r\n     
#define BK8000L_PAIRING_EXIT "CB"            //  Exit pairing  AT+CB\r\n     
#define BK8000L_CONNECT_LAST_DEVICE "CC"     //  The last paired device connected  AT+CC\r\n     what this should do? connect to last connected device?
#define BK8000L_DISCONNECT "CD"              //  disconnect  AT+CD\r\n     
#define BK8000L_CALL_ANSWARE "CE"            //  Answer the call   AT+CD\r\n     
#define BK8000L_CALL_REJECT "CF"             //  reject a call   AT+CF\r\n     
#define BK8000L_CALL_HANGUP "CG"             //  Hang up   AT+CG\r\n     
#define BK8000L_CALL_REDIAL "CH"             //  redial  AT+CH\r\n     last called number?
#define BK8000L_VOLUME_UP "CK"               //  volume up   AT+CK\r\n     
#define BK8000L_VOLUME_DOWN "CL"             //  volume down   AT+CL\r\n     
#define BK8000L_LANGUAGE_SWITCH  "CM"        //  Multi-language switch   AT+CM\r\n     
#define BK8000L_CHANNEL_SWITCH "CO"          //  Channel switching (invalid)   AT+CO\r\n     to be tested
#define BK8000L_SHUTDOWN "CP"                //  Shutdown  AT+CP\r\n     
#define BK8000L_SWITCH_INPUT "CT"            //  Enter the test mode   AT+CT\r\n     
#define BK8000L_OPEN_PHONE_VOICE "CV"        //  Open phone VOICE  AT+CV\r\n     
#define BK8000L_MEMORY_CLEAR "CZ"            //  Memory clear  AT+CZ\r\n     
#define BK8000L_LANGUAGE_SET_NUMBER "CMM"    //  Number:( 0-4 )  Set the number of multi-lingual   AT+CMM4\r\n     
#define BK8000L_MUSIC_TOGGLE_PLAY_PAUSE "MA" //  Music Play / Pause  AT+MA\r\n     
#define BK8000L_MUSIC_STOP "MC"              //  The music stops   AT+MC\r\n     
#define BK8000L_MUSIC_NEXT_TRACK "MD"        //  next track  AT+MD\r\n     
#define BK8000L_MUSIC_PREVIOUS_TRACK "ME"    //  previous track  AT+ME\r\n     
#define BK8000L_MUSIC_FAST_FORWARD "MF"      //  fast forward  AT+MF\r\n     test how does this exacly works?
#define BK8000L_MUSIC_REWIND "MH"            //  rewind  AT+MH\r\n     test how does this exacly works?
#define BK8000L_GET_NAME "MN"                //  Query bluetooth name  AT+MN\r\n   NA:BK8000L\r\n 
#define BK8000L_GET_CONNECTION_STATUS "MO"   //  Bluetooth connection status inquiry   AT+MO\rn  connection succeeded:" C1\r\n"no connection:"C0\r\n"  
#define BK8000L_GET_PIN_CODE "MP"            //  PIN Code query  AT+MP\r\n   PN:0000\r\n   
#define BK8000L_GET_ADDRESS "MR"             //  Query bluetooth address   AT+MR\r\n   AD:111111111111\r\n   
#define BK8000L_GET_SOFTWARE_VERSION "MQ"    //  Query software version  AT+MQ\r\n   XZX-V1.2\r\n  
#define BK8000L_MUSIC_GET_STATUS "MV "       //  Bluetooth playback status inquiry   AT+MV\r\n   Play: "MB\r\n", time out:"MA\r\n", disconnect:" M0\r\n"   
#define BK8000L_GET_HFP_STATUS "MY"          //  Bluetooth inquiry HFP status  AT+MY\r\n   disconnect:"M0\r\n", connection:"M1\r\n", Caller: "M2\r\n", Outgoing: "M3\r\n", calling:"M4\r\n"  

#ifdef BK8002
#define BK8002_SET_VOLUME "VOL" //Set the volume level AT+VOLx\r\n x=(0-15)
#define BK8002_BLUETOOTH_MODE "MNBT" //Bluetooth mode AT+MNBT\r\n
#define BK8002_SDCARD_MODE "MNMP3" //TF mode AT+MNMP3\r\n
#define BK8002_AUX_MODE "MNAUX" //AUX mode AT+MNAUX\r\n
#define BK8002_FM_MODE "MNFM" //FM mode AT+MNFM\r\n
#define BK8002_SDCARD_REPEAT_ALL "MPM0" //Loop all ( TF Mode) AT+MPM0\r\n
#define BK8002_SDCARD_REPEAT_ONE "MPM1" //Single loop playback ( TF Mode) AT+MPM1\r\n
#define BK8002_SDCARD_PLAY_SONG "SMP" //Play selection ( TF Mode) AT+SMPXXXX\r\n xxxx :( 0000-9999 ); (" 0000 "On behalf of the 1 first)
#define BK8002_GET_FM_CHANNEL_NUMBER "MRFM" //Inquire FM Channel number ( FM Mode) AT+MRFM\r\n; responce: FM99.8\r\n
#define BK8002_GET_SDCARD_SONG_NUMBER "MRMP3" //Inquire MP3 Song number ( TF Mode); AT+MRMP3\r\n; responce: MP3x\r\n
#define BK8002_GET_SDCARD_NUMBER_OF_SONGS "MMMP3" //Inquire MP3 Number of songs ( TF Mode); COM+MMMP3\r\n ; responce: MMPx\r\n (x=number of songs)
#define BK8002_GET_SDCARD_PLAY_MODE "MPMC" //Inquire MP3 Play mode ( TF Mode); AT+MPMC\r\n; responce: repreatall: PLAY_ALL\r\n; repeate one: PLAY_ONE\r\n
#define BK8002_GET_CURRENT_VOLUME "MVOL" //Query current volume; AT+MVOL\r\n ; responce: VOLx\r\n ( x : Represents the volume level)
#define BK8002_GET_CURRENT_MODE "MM" //Query current mode; AT+MM\r\n; responce: Bluetooth: BT_MODE\r\n TF : MP3_MODE\r\n FM : FM_MODE\r\n AUX : AUX_MODE\r\n
#endif

#if defined(USE_SW_SERIAL)
#if ARDUINO >= 100
#include <SoftwareSerial.h>
#else
#include <NewSoftSerial.h>
#endif
#endif


class BK8000L
{
  public:

    enum STATES
    {
      Playing, //MA
      Idle, //MB
      IncomingCall, //IR- or M2
      OutgoingCall, //PR- or M3
      CallInProgress, //M4
      Connected, // M1
      Disconnected, //M0
      On,
      Off,
      Pairing,
      ShutdownInProgress
    };

#ifdef BK8002
    enum modes
    {
	    BT,
	    FM,
	    SDCARD,
	    AUX,
	    ALL,
	    ONE
    };
    uint8_t playMode; //all or one 
    uint8_t mode; //BT,FM,AUX,SDCARD
    uint16_t songNumber;
    String CurrentFrequency; //send fro BT module as 99.8 so lets have it as string for now
#endif

    uint8_t BTState=Disconnected;
    uint8_t CallState=Disconnected;
    uint8_t MusicState=Idle;
    uint8_t PowerState=Off;
    String receivedSppData;

    String CallerID;
    String BT_ADDR;
    String BT_NAME;
    String BT_PIN;

#if defined(USE_SW_SERIAL)
#if ARDUINO >= 100
    BK8000L(SoftwareSerial *ser, uint8_t resetPin);
#else
    BK8000L(NewSoftSerial  *ser, uint8_t resetPin);
#endif
#else
    BK8000L(HardwareSerial *ser, uint8_t resetPin);
#endif
    void begin(uint32_t baudrate = 9600);
    ~BK8000L();

    uint8_t sendData(String cmd);
    uint8_t sendAPTData(String cmd);
    uint8_t aptLogin();

    uint8_t getNextEventFromBT();

    uint8_t pairingInit();
    uint8_t pairingExit();
    uint8_t connectLastDevice();
    uint8_t disconnect();
    uint8_t callAnsware();
    uint8_t callReject();
    uint8_t callHangUp();
    uint8_t callRedial();
    uint8_t volumeUp();
    uint8_t volumeDown();
    uint8_t languageSwitch();
    uint8_t channelSwitch();
    uint8_t shutdownBT();
    uint8_t switchInput();
    uint8_t openPhoneVoice();
    uint8_t memoryClear();
    uint8_t languageSetNumber(uint8_t number);
    uint8_t musicTogglePlayPause();
    uint8_t musicStop();
    uint8_t musicNextTrack();
    uint8_t musicPreviousTrack();
    uint8_t musicFastForward();
    uint8_t musicRewind();
    uint8_t getName();
    uint8_t getConnectionStatus();
    uint8_t getPinCode();
    uint8_t getAddress();
    uint8_t getSoftwareVersion();
    uint8_t getMusicStatus();
    uint8_t getHFPStatus();
    void resetModule();

#ifdef BK8002
   uint8_t setVolume(uint8_t volume); //BK8002_SET_VOLUME "VOL" //Set the volume level AT+VOLx\r\n x=(0-15)
   uint8_t setBluetoothMode(); //BK8002_BLUETOOTH_MODE "MNBT" //Bluetooth mode AT+MNBT\r\n
   uint8_t setSdcardMode(); //BK8002_SDCARD_MODE "MNMP3" //TF mode AT+MNMP3\r\n
   uint8_t setAUXMode(); //BK8002_AUX_MODE "MNAUX" //AUX mode AT+MNAUX\r\n
   uint8_t setFMMode(); //BK8002_FM_MODE "MNFM" //FM mode AT+MNFM\r\n
   uint8_t setSdcardRepeatAll(); //BK8002_SDCARD_REPEAT_ALL "MPM0" //Loop all ( TF Mode) AT+MPM0\r\n
   uint8_t setSdcardRepeaetOne(); //BK8002_SDCARD_REPEAT_ONE "MPM1" //Single loop playback ( TF Mode) AT+MPM1\r\n
   uint8_t setSdcardPlaSong(uint16_t songNumber); //BK8002_SDCARD_PLAY_SONG "SMP" //Play selection ( TF Mode) AT+SMPXXXX\r\n xxxx :( 0000-9999 ); (" 0000 "On behalf of the 1 first)
   uint8_t getFMChannelNumer(); //BK8002_GET_FM_CHANNEL_NUMBER "MRFM" //Inquire FM Channel number ( FM Mode) AT+MRFM\r\n; responce: FM99.8\r\n
   uint8_t getSdcardSongNumber(); //BK8002_GET_SDCARD_SONG_NUMBER "MRMP3" //Inquire MP3 Song number ( TF Mode); AT+MRMP3\r\n; responce: MP3x\r\n
   uint8_t getSdcardNumberOfSongs(); //BK8002_GET_SDCARD_NUMBER_OF_SONGS "MMMP3" //Inquire MP3 Number of songs ( TF Mode); COM+MMMP3\r\n ; responce: MMPx\r\n (x=number of songs)
   uint8_t getSdcardPlayMode(); //BK8002_GET_SDCARD_PLAY_MODE "MPMC" //Inquire MP3 Play mode ( TF Mode); AT+MPMC\r\n; responce: repreatall: PLAY_ALL\r\n; repeate one: PLAY_ONE\r\n
   uint8_t getCurrentVolume(); //BK8002_GET_CURRENT_VOLUME "MVOL" //Query current volume; AT+MVOL\r\n ; responce: VOLx\r\n ( x : Represents the volume level)
   uint8_t getCurrentMode(); //BK8002_GET_CURRENT_MODE "MM" //Query current mode; AT+MM\r\n; responce: Bluetooth: BT_MODE\r\n TF : MP3_MODE\r\n FM : FM_MODE\r\n AUX : AUX_MODE\r\n
#endif

  private:
    uint8_t _reset;

    String returnBtModuleName(String receivedString); //return module name between : and \0
    String returnCallerID(String receivedString); //return number between ""
    uint8_t decodeReceivedString(String receivedString);
    uint8_t checkResponce(void);
#if defined DEBUG
    void DBG(String text);
#endif
    void resetHigh();
    void resetLow();


#if  defined(USE_SW_SERIAL)
#if ARDUINO >= 100
    SoftwareSerial *btSerial;
#else
    NewSoftSerial  *btSerial;
#endif
#else
    HardwareSerial *btSerial;
#endif

};


#endif
