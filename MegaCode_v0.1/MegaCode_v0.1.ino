/**************************************************************
  RTC DS1307: https://github.com/Makuna/Rtc/
  GND - GND
  VCC - 5V
  SDA - 20
  SCL - 21
  DS -

  ESP-01S: https://github.com/blynkkk/blynk-library/releases/latest
  3V3 - 3V
  RX1 - TX1
  EN - 3V
  TX1 - RX1
  GND - GND

  NANO:
  D6 - RX2
  D5 TX2
  GND - GND

  RGB LED:
  R - 2
  G - 3
  B - 4
  GND - GND

  DHT22:  https://github.com/adafruit/DHT-sensor-library
  "+" - 5V
  OUT - 5
  "-" - GND

  HCSR505:
  "+" - 5V
  OUT - 6
  "-" - GND

  SERVO:
  GND - GND
  5V - 5V
  OUT - 7


  TFT:  https://github.com/MickThomson21/UTFT-ILI9225
  5V - 5V
  GND - GND
  GND -
  NC - VIN
  NC -
  LED - A0
  SCL - A1
  SDA - A2
  RS - A3
  RST - A4
  CS - A5
  SD_CS - 8
  SD_MOSI - 51
  SD_SCK - 52
  SD_MISO - 50

  TOUCH:
  GND - GND
  OUT - 9
  VCC - 5V

  Door Magnetic sensor:
  GND - GND
  OUT - 10

  JQ6500: https://github.com/sleemanj/JQ6500_Serial
  RX+1k(L1) - 10
  TX(L2) - 11
  GND(L3) - GND
  DC-5V(L4) - 5V
  SPK+(L7) - SPK+
  SPK-(L8) - SPK-

  MFRC522: https://github.com/miguelbalboa/rfid
  ->NANO

 *************************************************************/

#define BLYNK_PRINT Serial
#define EspSerial Serial1
#define BLYNK_MAX_READBYTES 1024
#define BLYNK_EXPERIMENTAL
#define JQ6500Serial Serial2
#define NanoSerial Serial3

#define LED_RED 2
#define LED_GREEN 3
#define LED_BLUE 4
#define LED_RGB 0
#define FOREVER 0
#define LED_RGBLOOP_TYPE1 -1
#define LED_RGBLOOP_TYPE2 -2
#define LEDSTATE_CHANGEABLE HIGH
#define LEDSTATE_UNCHANGEABLE LOW
#define LED_ON HIGH
#define LED_OFF LOW
#define SHORTTIME 300
#define LONGTIME 1000

#define DHTPIN 5

#define HCSR505PIN 6
#define FINDSOMEBODY HIGH
#define NOBODY LOW

#define SERVOPIN 7
#define LOCKED HIGH
#define UNLOCKED LOW

#define SDPIN 8

#define TOUCHPIN 9
#define TOUCHED HIGH
#define UNTOUCHED LOW

#define DOORPIN 10
#define OPENED HIGH
#define CLOSED LOW



#include <ESP8266_Lib.h>  // https://github.com/blynkkk/blynk-library/releases/latest
#include <BlynkSimpleShieldEsp8266.h> // https://github.com/blynkkk/blynk-library/releases/latest
#include <Wire.h> // must be included here so that Arduino library object file references work
#include <RtcDS1307.h>  //https://github.com/Makuna/Rtc/
#include <DHT.h>  // https://github.com/adafruit/DHT-sensor-library
#include <Servo.h>
#include <UTFT.h> //https://github.com/MickThomson21/UTFT-ILI9225
#include <UTFT_DLB.h> //https://github.com/steelice/UTFT_DLB
#include <SPI.h>
#include <SD.h>
#include <qrcode.h> //https://github.com/ricmoo/QRCode
#include <Hash.h> //https://github.com/mr-glt/Arduino-SHA-1-Hash
#include <EEPROM.h>
#include <JQ6500_Serial_HardwareSerial.h>  //https://github.com/lightcalamar/JQ6500_Serial_HardwareSerial



char auth[] = "60c1636fb25b47d1900311a000503a01"; //blynk project auth tockens
char ssid[] = "USR-G800-118D";
char pass[] = "123456780";
ESP8266 wifi(&EspSerial);
BlynkTimer timer;	//http://playground.arduino.cc/Code/SimpleTimer#Usage
uint8_t FBtimer, CLtimer;	//findBody, checkLocker

File Log;
RtcDS1307<TwoWire> Rtc(Wire);
DHT dht(DHTPIN, DHT22);
Servo myservo;
JQ6500_Serial_HardwareSerial mp3(JQ6500Serial);
uint8_t VOLUME = 30;
bool TEST = true;
bool LED_RED_STATE = LEDSTATE_CHANGEABLE, LED_GREEN_STATE = LEDSTATE_CHANGEABLE, LED_BLUE_STATE = LEDSTATE_CHANGEABLE;
bool LockStage = LOCKED, LockedByApp = true;
bool writedtlog = true, writetemlog = true, writehumlog = true;
UTFT_DLB myGLCD(QD220A, A2, A1, A5, A4, A3);
extern uint8_t BigFont[];
//extern uint8_t SmallFont[];
//extern uint8_t SevenSegNumFont[];
//extern uint8_t DejaVuSans18[];
extern uint8_t DejaVuSans24[];
extern uint8_t BeynoBlackPanther24[]; //logos
extern uint8_t  BVS_13[]; //logs
extern uint8_t  Farrington7BQiqi16[]; //idnums
extern uint8_t  Lcd2u24[];



void setup() {
  Serial.begin(115200);
  Serial.println("==>setup----------");

  EspSerial.begin(115200);
  delay(10);
  Blynk.begin(auth, wifi, ssid, pass, "192.168.1.5", 8080);

  JQ6500Serial.begin(9600);
  mp3.reset();
  mp3.setVolume(VOLUME);

  setDevicesId();

  NanoSerial.begin(9600);

  myGLCD.InitLCD();
  myGLCD.clrScr();

  setDataTime();

  dht.begin();

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);

  pinMode(SDPIN, OUTPUT);

  pinMode(HCSR505PIN, INPUT);

  pinMode(TOUCHPIN, INPUT);

  myservo.attach(SERVOPIN);
  pinMode(SERVOPIN, OUTPUT);

  pinMode(DOORPIN, INPUT);

  if (TEST) {
    hardwareTest();
    TEST = !TEST;
  }

  lockLocker();

  FBtimer = timer.setInterval(1000, findBody);
  CLtimer = timer.setInterval(1000, checkLocker);

  Serial.println("==>setup-----Done!");
}


void loop() {
  Blynk.run();
  timer.run();
}


void hardwareTest() {
  Serial.println("==>hardwareTest----------");
  drawQrcode("http://yit.edu.cn");
  delay(2000);
  displayPictureInSdcard("yitlogo.raw");
  delay(2000);
  testRtc();
  testDht();
  //testRgbLed();
  //testServo();
  //testTouch();
  //testmp3();
  Serial.println("==>hardwareTest-----Done!");
}


void testRtc() {
  Serial.println("==>testRtc----------");
  setDataTime();
  displayDataAndTime();
  Serial.println("==>testRtc-----Done!");
}


void testDht() {
  Serial.println("==>testDht----------");
  displayTemperature();
  displayHumidity();
  Serial.println("==>testDht-----Done!");
}


void testRgbLed() {
  Serial.println("==>testRgbLed----------");
  ledBlink(LED_RED, 3, SHORTTIME);
  ledBlink(LED_GREEN, 3, LONGTIME);
  ledBlink(LED_BLUE, 3, SHORTTIME);
  ledBlink(LED_RGBLOOP_TYPE1, 3, LONGTIME);
  ledBlink(LED_RGBLOOP_TYPE2, 3, SHORTTIME);
  ledSolid(LED_RED, 3000);
  Serial.println("==>testRgbLed-----Done!");
}


void testServo() {
  Serial.println("==>testServo----------");
  lockLocker();
  delay(2000);
  unlockLocker();
  delay(2000);
  Serial.println("==>testServo-----Done!");
}


void testTouch() {
  if (digitalRead(TOUCHPIN) == TOUCHED) {
    Serial.println("TOUCHED");
    mp3.playFileByIndexNumber(11);
  }
  else {
    Serial.println("UNTOUCHED");
  }
}


void testmp3() {
  mp3.playFileByIndexNumber(1);  //管理卡添加完成
  delay(2000);
  mp3.playFileByIndexNumber(2);  //已进入管理模式
  delay(2000);
  mp3.playFileByIndexNumber(3);  //已退出管理模式
  delay(2000);
  mp3.playFileByIndexNumber(4);  //新卡添加完成
  delay(2000);
  mp3.playFileByIndexNumber(5);  //删除完成
  delay(2000);
  mp3.playFileByIndexNumber(6);  //屌丝请进
  delay(2000);
  mp3.playFileByIndexNumber(7);  //管理员请进
  delay(2000);
  mp3.playFileByIndexNumber(8);  //土豪请进
  delay(2000);
  mp3.playFileByIndexNumber(9);  //余额不足
  delay(2000);
  mp3.playFileByIndexNumber(11);  //dong ong ong ong ong ong
  delay(2000);
  mp3.setLoopMode(MP3_LOOP_ONE);
  mp3.playFileByIndexNumber(10);  //beep beep beep  beep beep beep
  delay(5000);
  mp3.setLoopMode(MP3_LOOP_ONE_STOP);
}



#define MULTIPLIER 3
void drawQrcode(const char* url) {
  Serial.println("==>drawQrcode----------");
  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(9)];
  qrcode_initText(&qrcode, qrcodeData, 9, 0, url);
  myGLCD.InitLCD();
  myGLCD.fillScr(255, 255, 255);
  for (uint8_t y = 0; y < qrcode.size; y++) {
    for (uint8_t x = 0; x < qrcode.size; x++) {
      if (qrcode_getModule(&qrcode, x, y)) {
        myGLCD.setColor(0, 0, 0);
        myGLCD.fillRect(MULTIPLIER * x, MULTIPLIER * y, MULTIPLIER * x + MULTIPLIER, MULTIPLIER * y + MULTIPLIER);
      }
    }
  }
  Serial.println("==>drawQrcode-----Done!");
}



BLYNK_READ(V0) { //transDevicesID
  Blynk.virtualWrite(V0, eepromLoadString(0));
}
void setDevicesId() {
  if (digitalRead(TOUCHPIN) == TOUCHED) {
    Serial.println("Stop touching now then input DevicesID");
    Serial.println("Or you will quit set DevicesID after 3 seconds");
    if (touchedTime() > 3000) {
      Serial.println("Quit set DevicesID");
      return;
    }
    Serial.println("Please input a string, it will be set as DevicesID:");
    Serial.println("Or you can input 'Q' to quit set DevicesID");
    while (1) {
      if (Serial.available()) {
        String str = Serial.readString();
        if (str == "Q" || str == "q") {
          Serial.println("Quit set DevicesID");
          return;
        }
        char* devicesID = const_cast<char*>(str.c_str());
        Serial.print("Set DevicesID as: "); Serial.print(str); Serial.println(" ?");
        Serial.println("Re input DevicesID again for commit");
        while (1) {
          if (Serial.available()) {
            String str2 = Serial.readString();
            if (str == str2) {
              eepromSaveString(devicesID, 0);
              Serial.print("DevicesID had been seted as: "); Serial.println(eepromLoadString(0));
              Serial.println("==>setDevicesId-----Done!");
              return;
            }
            else {
              Serial.println("ERROR: Re INPUT DIFFERENT!!!");
              Serial.println("Please input a string, it will be set as DevicesID:");
              break;
            }
          }
        }
      }
    }
  }
}
uint8_t touchedTime() {
  uint8_t touchedtime = 0;
  if (digitalRead(TOUCHPIN) == TOUCHED) {
    Serial.println("Touched");
    mp3.playFileByIndexNumber(11);
    uint32_t starttime = millis();
    while (digitalRead(TOUCHPIN) == TOUCHED) {
      touchedtime = millis() - starttime;
      if (touchedtime > 3000) {
        return 3001;
      }
    }
  }
  return touchedtime;
}
uint8_t count = 3; //https://github.com/filip505/EEPROM-String
void eepromSaveString(char *data, uint8_t id) {
  uint8_t dataSize = strlen(data);
  EEPROM.write(id, dataSize);
  uint8_t start = 0;
  for (uint8_t i = 0; i < id; i++)
  {
    start += EEPROM.read(i);
  }
  dataSize = dataSize + count + start;
  for (uint8_t i = count + start; i < dataSize; i++)
  {
    EEPROM.write(i, data[i - count - start]);
  }
}
String eepromLoadString(uint8_t id) {
  uint8_t dataSize = EEPROM.read(id);
  String rez;
  uint8_t endNum = dataSize + count;
  uint8_t start = 0;
  for (uint8_t i = 0; i < id; i++)
  {
    start += EEPROM.read(i);
  }
  endNum += start;
  for (uint8_t i = count + start; i < endNum; i++)
  {
    rez += char(EEPROM.read(i));
  }
  return rez;
}


BLYNK_READ(V1) {  //transData  e.g. 2018/06/22 W:5
  RtcDateTime dt = Rtc.GetDateTime();
  char datebufwithyear[20];
  sprintf( datebufwithyear, "%04u/%02u/%02u", dt.Year(), dt.Month(), dt.Day() );
  uint8_t week = calculateWeek( dt.Year(), dt.Month(), dt.Day() );
  String transdata = String(datebufwithyear) + "  W:" + String(week) ;
  Blynk.virtualWrite(V1, transdata);
}
uint8_t calculateWeek(uint8_t y, uint8_t m, uint8_t d) {
  if (m == 1) m = 13;
  if (m == 2) m = 14;
  return (d + 2 * m + 3 * (m + 1) / 5 + y + y / 4 - y / 100 + y / 400) % 7 + 1;
}


BLYNK_READ(V2) {  //transTime  e.g. 08:00
  RtcDateTime dt = Rtc.GetDateTime();
  char timebuf[20];
  sprintf( timebuf, "%02u:%02u", dt.Hour(), dt.Minute() );
  String transtime = String(timebuf);
  Blynk.virtualWrite(V2, transtime);
}


BLYNK_READ(V3) {  //transRunTime  e.g. 123
  Blynk.virtualWrite(V3, millis() / 1000);
}


BLYNK_READ(V4) {  //transTemperature  e.g. 26.5
  Blynk.virtualWrite(V4, dht.readTemperature());
}


BLYNK_READ(V5) {  //transHumidity  e.g. 56,5
  Blynk.virtualWrite(V5, dht.readHumidity());
}


WidgetLED bodyLED(V6); //transFindBody
void findBody() {
  if (digitalRead(HCSR505PIN) == FINDSOMEBODY) {
    //Serial.println();
    //Serial.println("Find somebody");
    //ledSolid(LED_BLUE, FOREVER);
    mp3.playFileByIndexNumber(11);
    bodyLED.on();
    myGLCD.InitLCD();
    displayDefaultInterface();
  }
  else {
    //ledTurnOff(LED_BLUE);
    bodyLED.off();
  }
}


BLYNK_WRITE(V9) {	//lockByApp
  if (param.asInt()) {
    lockLocker();
    LockedByApp = true;
  }
  else {
    unlockLocker();
    LockedByApp = false;
  }
}


WidgetLED lockLED(V7); //transLockStage
void changeLockStage() {
  Serial.println("==>changeLockStage----------");
  if (LockedByApp) {
    Serial.println("locked by app");
    Serial.println("==>changeLockStage-----Denined!");
    ledBlink(LED_RED, 1, LONGTIME);
  }
  else {
    Serial.println("lockstage changeable");
    timer.disable(CLtimer); timer.disable(FBtimer);
    if (LockStage == LOCKED) {
      unlockLocker();
    }
    else {
      lockLocker();
    }
    Serial.println("==>changeLockStage-----Done!");
    timer.enable(CLtimer); timer.enable(FBtimer);
  }
}
void lockLocker() {
  myservo.attach(SERVOPIN);
  myservo.write(100);
  ledBlink(LED_RED, 1, LONGTIME);
  timer.setTimeout(800L, []() {
    myservo.detach();
  });
  LockStage = LOCKED;
  lockLED.on();
  Serial.println("Locked :)");
}
void unlockLocker() {
  myservo.attach(SERVOPIN);
  myservo.write(0);
  ledBlink(LED_GREEN, 1, LONGTIME);
  timer.setTimeout(800L, []() {
    myservo.detach();
  });
  LockStage = UNLOCKED;
  lockLED.off();
  Serial.println("Unlocked :(");
}


WidgetLED doorLED(V8); //transDoorStage
bool checkDoor() {
  bool doorstate = digitalRead(DOORPIN);
  if (doorstate == OPENED) {
    Serial.println();
    Serial.println("Door opened");
    doorLED.on();
    mp3.playFileByIndexNumber(11);
  }
  else {
    doorLED.off();
  }
  return doorstate;
}


BLYNK_WRITE(V10) {	//changeVolumeByApp
  VOLUME = param.asInt();
  Serial.print("volume: ");
  Serial.print(VOLUME);
  mp3.setVolume(VOLUME);
  mp3.playFileByIndexNumber(11);
  Serial.println("==>setvolume-----Done!");
}


BLYNK_WRITE(V11) {	//changeRedLedHintableByApp
  if (param.asInt() == 0) {
    ledTurnOff(LED_RED);
    ledBlink(LED_RED, 2, SHORTTIME);
    LED_RED_STATE = LEDSTATE_UNCHANGEABLE;
  }
  else {
    LED_RED_STATE = LEDSTATE_CHANGEABLE;
    ledBlink(LED_RED, 2, SHORTTIME);
  }
}


BLYNK_WRITE(V12) {	//changeGreenLedHintableByApp
  if (param.asInt() == 0) {
    ledTurnOff(LED_GREEN);
    ledBlink(LED_GREEN, 2, SHORTTIME);
    LED_GREEN_STATE = LEDSTATE_UNCHANGEABLE;
  }
  else {
    LED_GREEN_STATE = LEDSTATE_CHANGEABLE;
    ledBlink(LED_GREEN, 2, SHORTTIME);
  }
}


BLYNK_WRITE(V13) {	//changeBlueLedHintableByApp
  if (param.asInt() == 0) {
    ledTurnOff(LED_BLUE);
    ledBlink(LED_BLUE, 2, SHORTTIME);
    LED_BLUE_STATE = LEDSTATE_UNCHANGEABLE;
  }
  else {
    LED_BLUE_STATE = LEDSTATE_CHANGEABLE;
    ledBlink(LED_BLUE, 2, SHORTTIME);
  }
}


void checkLocker() {
  if (NanoSerial.available()) {
    String command = NanoSerial.readString();
    Serial.println(command);
    if (command == "Welcome") {
      changeLockStage();
    }
  }
}


void setDataTime() {
  Rtc.Begin();
  RtcDateTime dt = RtcDateTime(__DATE__, __TIME__);
  if (!Rtc.IsDateTimeValid()) {
    Serial.println("ERROR: RTC LOW BATTERY!!!");
    myGLCD.setColor(255, 255, 255); myGLCD.setBackColor(0, 0, 0); myGLCD.setFont(BVS_13);
    myGLCD.print("ERROR: RTC LOW BATTERY!!!", 10, 10, 0);
  }
  if (!Rtc.GetIsRunning()) {
    Rtc.SetIsRunning(true);
  }
  Rtc.SetDateTime(dt);
  Rtc.SetSquareWavePin(DS1307SquareWaveOut_Low);
}


void displayPictureInSdcard(String PictureName) {
  Serial.println("==>displayPictureInSdcard----------");
  if (SD.begin(SDPIN)) {
    File rawpicture = SD.open(PictureName, FILE_READ);
    myGLCD.InitLCD();
    uint8_t r, g, b;
    for (uint8_t y = 0; y < 176; y ++) {
      for (uint8_t x = 0; x < 220; x++) {
        r = rawpicture.read();
        g = rawpicture.read();
        b = rawpicture.read();
        myGLCD.setColor(r, g, b);
        myGLCD.drawPixel(x, y);
      }
    }
    rawpicture.close();
  }

  Serial.println("==>displayPictureInSdcard-----Done!");
}


#define BGSTR1_POSITION_X 53  //45
#define BGSTR1_POSITION_Y 65  //65
#define BGSTR2_POSITION_X 53  //53
#define BGSTR2_POSITION_Y 95  //95
void displayBackgroundText() {
  String bgstr1 = "Y I T", bgstr2 = "BO CHUANG";
  myGLCD.fillScr(20, 20, 20);
  //myGLCD.setColor(50, 50, 50);
  //myGLCD.fillRect(30, 30, 220 - 30, 176 - 30);
  myGLCD.setColor(155, 0, 255); myGLCD.setBackColor(20, 20, 20); myGLCD.setFont(BeynoBlackPanther24);	//瓦坎达紫
  //myGLCD.drawRect(30, 30, 220 - 30, 176 - 30);
  myGLCD.print(bgstr1, BGSTR1_POSITION_X, BGSTR1_POSITION_Y, 0); myGLCD.print(bgstr2, BGSTR2_POSITION_X, BGSTR2_POSITION_Y, 0);
}


void displayDefaultInterface() {
  Serial.println("==>displayDefaultInterface----------");
  timer.disable(CLtimer); timer.disable(FBtimer);
  displayBackgroundText();
  writedtlog = true; writetemlog = true; writehumlog = true;
  //myGLCD.fillScr(0, 0, 100);
  displayDataAndTime(); displayTemperature(); displayHumidity();
  timer.enable(CLtimer);
  timer.setTimeout(10000, []() {
    myGLCD.clrScr();
    timer.enable(FBtimer);
    Serial.println("==>displayDefaultInterface-----Done!");
  } );
}


#define DATA_POSITION_X 135
#define DATA_POSITION_Y 5
#define TIME_POSITION_X DATA_POSITION_X
#define TIME_POSITION_Y DATA_POSITION_Y+20
#define WEEK_POSITION_X DATA_POSITION_X+3*15
#define WEEK_POSITION_Y TIME_POSITION_Y+20
void displayDataAndTime() {
  RtcDateTime dt2 = Rtc.GetDateTime();
  if (!Rtc.IsDateTimeValid()) {
    Serial.println("ERROR: RTC LOW BATTERY!!!");
    myGLCD.setColor(255, 255, 255); myGLCD.setBackColor(0, 0, 0); myGLCD.setFont(BVS_13);
    myGLCD.print("ERROR: RTC LOW BATTERY!!!", 10, 10, 0);
  }
  char datebuf[20], datebufwithyear[20], timebuf[20], timebufwithsecond[20];
  sprintf( datebuf, "%02u/%02u", dt2.Month(), dt2.Day() );
  sprintf( datebufwithyear, "%04u/%02u/%02u", dt2.Year(), dt2.Month(), dt2.Day() );
  sprintf( timebuf, "%02u:%02u", dt2.Hour(), dt2.Minute() );
  sprintf( timebufwithsecond, "%02u:%02u:%02u", dt2.Hour(), dt2.Minute(), dt2.Second() );
  uint8_t week = calculateWeek( dt2.Year(), dt2.Month(), dt2.Day() );
  if (writedtlog == true) {
    String dtlog = "DevicesID: " + eepromLoadString(0) + "  Data: " + String(datebufwithyear) + "  Week: " + String(week) + "  Time: " + String(timebufwithsecond);
    String sha1dtlog = "SHA1_dtlog: " + String(sha1(dtlog));
    Serial.println(dtlog);
    Serial.println(sha1dtlog);
    Log = SD.open("Log.txt", FILE_WRITE);
    if (Log) {
      //Log.println(dtlog);
      Log.println(sha1dtlog);
      Log.close();
    }
    else
    {
      Serial.println("ERROR: OPEN LOG.TXT");
    }
    writedtlog = false;
  }
  myGLCD.setColor(255, 255, 255); myGLCD.setBackColor(0, 0, 0); myGLCD.setFont(BigFont);
  myGLCD.print(datebuf, DATA_POSITION_X, DATA_POSITION_Y, 0);
  myGLCD.print(timebuf, TIME_POSITION_X, TIME_POSITION_Y, 0);
  myGLCD.print("." + String(week), WEEK_POSITION_X, WEEK_POSITION_Y);
}


#define TEMPERATURE_POSITION_X 5
#define TEMPERATURE_POSITION_Y DATA_POSITION_Y
void displayTemperature() {
  float temperature = dht.readTemperature();
  if ( isnan(temperature) ) {
    Serial.println("ERROR: READ TEMPERATURE!!!");
    myGLCD.setColor(255, 255, 255); myGLCD.setBackColor(0, 0, 0); myGLCD.setFont(BVS_13);
    myGLCD.print("ERROR: READ TEMPERATURE!!!", 10, 30, 0);
  }
  else {
    if (writetemlog == true) {
      String temlog = "Temperature: " + String(temperature) + " *C";
      Serial.println(temlog);
      Log = SD.open("Log.txt", FILE_WRITE);
      if (Log) {
        Log.println(temlog);
        Log.close();
      }
      else
      {
        Serial.println("ERROR: OPEN LOG.TXT");
      }
      writetemlog = false;
    }
    uint8_t inttemperature = int(temperature);
    graphicalTemperature(inttemperature);
    myGLCD.setBackColor(0, 0, 0); myGLCD.setFont(BigFont);
    myGLCD.print(String(inttemperature) + "*C", TEMPERATURE_POSITION_X, TEMPERATURE_POSITION_Y);
  }
}


void graphicalTemperature(uint8_t inttemperature) {
  myGLCD.setColor(150, 150, 150);  myGLCD.setBackColor(0, 0, 0);
  myGLCD.fillCircle(12, 165, 5); myGLCD.fillRoundRect(10, 65, 15, 165);
  if (inttemperature <= 10) {
    myGLCD.setColor(30, 30, 220); //深蓝
    Serial.println("WARNING: LOW TEMPERATURE !!!");
  }
  else if (inttemperature > 10 && inttemperature <= 25) {
    myGLCD.setColor(0, 200, 255); //浅蓝
  }
  else if (inttemperature > 25 && inttemperature <= 32) {
    myGLCD.setColor(255, 255, 0); //金黄
  }
  else if (inttemperature > 32 && inttemperature <= 40) {
    myGLCD.setColor(255, 200, 0); //橙黄
  }
  else if (inttemperature > 40 && inttemperature <= 50) {
    myGLCD.setColor(255, 0, 0); //红
  }
  else {
    myGLCD.setColor(255, 0, 0); //红
    Serial.println("WARNING: HIGH TEMPERATURE !!!");
  }
  myGLCD.fillCircle(12, 165, 4); myGLCD.fillRect(11, 165 - inttemperature, 14, 165);
}


#define HUMIDITY_POSITION_X TEMPERATURE_POSITION_X
#define HUMIDITY_POSITION_Y TEMPERATURE_POSITION_Y+20
void displayHumidity() {
  float humidity = dht.readHumidity();
  if ( isnan(humidity) ) {
    Serial.println("ERROR: READ HUMIDITY!!!");
    myGLCD.setColor(255, 255, 255); myGLCD.setBackColor(0, 0, 0); myGLCD.setFont(BVS_13);
    myGLCD.print("ERROR: READ HUMIDITY!!!", 10, 30, 0);
  }
  else {
    if (writehumlog == true) {
      String humlog = "Humidity: " + String(humidity) + " %";
      Serial.println(humlog);
      Log = SD.open("Log.txt", FILE_WRITE);
      if (Log) {
        Log.println(humlog);
        Log.close();
      }
      else
      {
        Serial.println("ERROR: OPEN LOG.TXT");
      }
      writehumlog = false;
    }
    uint8_t inthumidity = int(humidity);
    graphicalHumidity(inthumidity);
    myGLCD.setBackColor(0, 0, 0); myGLCD.setFont(BigFont);
    myGLCD.print(String(inthumidity) + "%", HUMIDITY_POSITION_X, HUMIDITY_POSITION_Y);
  }
}

void graphicalHumidity(uint8_t inthumidity) {
  myGLCD.setColor(150, 150, 150);  myGLCD.setBackColor(0, 0, 0);
  myGLCD.fillCircle(27, 165, 5); myGLCD.fillRoundRect(25, 65, 30, 165);
  if (inthumidity <= 45) {
    myGLCD.setColor(0, 200, 255); //浅蓝
  }
  else if (inthumidity > 45 && inthumidity <= 75) {
    myGLCD.setColor(255, 255, 0); //金黄
  }
  else {
    myGLCD.setColor(255, 0, 0); //红
  }
  myGLCD.fillCircle(27, 165, 4); myGLCD.fillRect(26, 165 - inthumidity, 29, 165);
}


void ledTurnOff(uint8_t LED_X) {
  if (LED_X == LED_RGB) {
    digitalWrite(LED_RED, LED_OFF);
    digitalWrite(LED_GREEN, LED_OFF);
    digitalWrite(LED_BLUE, LED_OFF);
  }
  digitalWrite(LED_X, LED_OFF);
}


bool checkLedState(uint8_t LED_X) {
  if (LED_X == LED_RED) {
    return LED_RED_STATE;
  }
  else if (LED_X == LED_GREEN) {
    return LED_GREEN_STATE;
  }
  else if (LED_X == LED_BLUE) {
    return LED_BLUE_STATE;
  }
  else {
    return LEDSTATE_CHANGEABLE;
  }
}


void ledSolid(uint8_t LED_X, uint8_t delaytime) {
  if (checkLedState(LED_X) == LEDSTATE_UNCHANGEABLE) {
    return;
  }
  ledTurnOff(LED_X);
  digitalWrite(LED_X, LED_ON);
  if (delaytime == FOREVER) {
    return;
  }
  Blynk_Delay(delaytime);
  digitalWrite(LED_X, LED_OFF);
}


void ledBlink(uint8_t LED_X, uint8_t times, uint8_t delaytime) {
  if (checkLedState(LED_X) == LEDSTATE_UNCHANGEABLE) {
    return;
  }
  ledTurnOff(LED_X);
  if (LED_X == LED_RGBLOOP_TYPE1) {
    for (uint8_t i = 0; i < (times); i++) {
      digitalWrite(LED_RED, LED_ON);
      Blynk_Delay(delaytime);
      digitalWrite(LED_RED, LED_OFF);
      digitalWrite(LED_GREEN, LED_ON);
      Blynk_Delay(delaytime);
      digitalWrite(LED_GREEN, LED_OFF);
      digitalWrite(LED_BLUE, LED_ON);
      Blynk_Delay(delaytime);
      digitalWrite(LED_BLUE, LED_OFF);
    }
    return;
  }
  if (LED_X == LED_RGBLOOP_TYPE2) {
    for (uint8_t i = 0; i < (times); i++) {
      digitalWrite(LED_RED, LED_ON);
      Blynk_Delay(delaytime);
      digitalWrite(LED_RED, LED_OFF);
      Blynk_Delay(delaytime);
      digitalWrite(LED_GREEN, LED_ON);
      Blynk_Delay(delaytime);
      digitalWrite(LED_GREEN, LED_OFF);
      Blynk_Delay(delaytime);
      digitalWrite(LED_BLUE, LED_ON);
      Blynk_Delay(delaytime);
      digitalWrite(LED_BLUE, LED_OFF);
      Blynk_Delay(delaytime);
    }
    return;
  }
  bool LED_STATE = LED_ON;
  for (uint8_t i = 0; i < (times * 2); i++) {
    digitalWrite(LED_X, LED_STATE);
    LED_STATE = !LED_STATE;
    Blynk_Delay(delaytime);
  }
}


void Blynk_Delay(uint8_t delaytime) {
  uint32_t starttime = millis();
  while (millis() - starttime < delaytime) {
    if (!TEST) {
      Blynk.run();
      timer.run();
    }
  }
}
