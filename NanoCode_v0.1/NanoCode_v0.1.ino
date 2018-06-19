/**************************************************************
  RGB LED:
  R - D2
  G - D3
  B - D4
  GND - GND
  
  MEGA:
  TX2 - D5
  RX2 - D6
  GND - GND
  
  TOUCH:
  VCC - 5V
  OUT - D8
  GND - GND
  
  
  MFRC522: https://github.com/miguelbalboa/rfid
  SDA - D10
  SCK - D13
  MOSI - D11
  MISO - D12
  IRQ -
  GND - GND
  RST - D9
  3.3V - 3.3V
  
*************************************************************/



#define LED_RED 2
#define LED_GREEN 3
#define LED_BLUE 4
#define LED_RGB 0
#define LED_RGBLOOP_TYPE1 -1
#define LED_RGBLOOP_TYPE2 -2
#define LED_ON HIGH
#define LED_OFF LOW
#define SHORTTIME 300
#define LONGTIME 1000

#define SOFTRXPIN 5
#define SOFTTXPIN 6

#define TOUCHPIN 8

#define MFRC522_RST_PIN 9
#define MFRC522_SS_PIN 10



#include <SPI.h>
#include <MFRC522.h>  // https://github.com/miguelbalboa/rfid
MFRC522 mfrc522(MFRC522_SS_PIN, MFRC522_RST_PIN);
#include <SoftwareSerial.h>
SoftwareSerial mySerial(SOFTRXPIN, SOFTTXPIN);
#include <EEPROM.h>



bool programMode = false;
uint8_t successRead;    // Variable integer to keep if we have Successful Read from Reader
byte readCard[4];   // Stores scanned ID read from RFID Module
byte masterCard[4];   // Stores master card's ID read from EEPROM
byte storedCard[4];   // Stores an ID read from EEPROM



void setup() {
  Serial.begin(115200);
  Serial.println("==>setup----------");
  
  SPI.begin();
  mfrc522.PCD_Init();
  ShowReaderDetails();
  
  mySerial.begin(9600);
  
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  
  pinMode(TOUCHPIN, INPUT);
  touchToDoSomething();

  if (EEPROM.read(1) != 233) {
    Serial.println(F("No Admin"));
    Serial.println(F("Scan a PICC to define as MasterCard"));
    do {
      successRead = getID();            // sets successRead to 1 when we get read from reader otherwise 0
      ledBlink(LED_BLUE, 2, SHORTTIME);
    }
    while (!successRead);                  // Program will not go further while you not get a successful read
    for ( uint8_t j = 0; j < 4; j++ ) {        // Loop 4 times
      EEPROM.write( 2 + j, readCard[j] );  // Write scanned PICC's UID to EEPROM, start from address 3
    }
    EEPROM.write(1, 233);                  // Write to EEPROM we defined Master Card.
    Serial.println(F("MasterCard defined"));
  }

  Serial.println(F("-------------------"));
  Serial.println(F("MasterCard's UID"));
  for ( uint8_t i = 0; i < 4; i++ ) {          // Read Master Card's UID from EEPROM
    masterCard[i] = EEPROM.read(2 + i);    // Write it to masterCard
    Serial.print(masterCard[i], HEX);
  }
  Serial.println("==>setup-----Done!");
}



void loop () {
	
  do {
    successRead = getID();  // sets successRead to 1 when we get read from reader otherwise 0
    if (digitalRead(TOUCHPIN) == HIGH) { // Check if button is pressed
      touchToDoSomething();
    }
    if (programMode) {
      ledTurnOn(LED_BLUE);
    }
    else {
      ledTurnOff(LED_RED);
    }
  } while (!successRead);   //the program will not go further while you are not getting a successful read
  
  if (programMode) {
    if ( isMaster(readCard) ) { //When in program mode check First If master card scanned again to exit program mode
      ledTurnOff(LED_RGB);
      Serial.println(F("MasterCard scanned"));
      Serial.println(F("Exiting ProgramMode"));
      Serial.println(F("-----------------------------"));
      programMode = false;
      return;
    }
    else {
      if ( findID(readCard) ) { // If scanned card is known delete it
        Serial.println(F("I know this PICC, removing..."));
        delateUser(readCard);
        hintScan();
      }
      else {                    // If scanned card is not known add it
        Serial.println(F("I do not know this PICC, adding..."));
        addUser(readCard);
        hintScan();
      }
    }
  }
  else {
    if ( isMaster(readCard)) {    // If scanned card's ID matches Master Card's ID - enter program mode
      programMode = true;
      Serial.println(F("Hello master - Entered ProgramMode"));
      uint8_t count = EEPROM.read(0);   // Read the first Byte of EEPROM that
      Serial.print(F("I have "));     // stores the number of ID's in EEPROM
      Serial.print(count);
      Serial.print(F(" record(s) on EEPROM"));
      Serial.println("");
      hintScan();
      Serial.println(F("Scan MasterCard again to exit ProgramMode"));
      Serial.println(F("-----------------------------"));
    }
    else {
      if ( findID(readCard) ) { // If not, see if the card is in the EEPROM
        Serial.println(F("Welcome, You shall pass"));
        mySerial.print("Welcome");
        ledBlink(LED_GREEN, 1, LONGTIME);
      }
      else {      // If not, show that the ID was not valid
        Serial.println(F("Stranger, You shall not pass"));
        mySerial.print("Stranger");
        ledBlink(LED_RED, 1, LONGTIME);
      }
    }
  }
}



void hintScan() {
  Serial.println(F("-----------------------------"));
  Serial.println(F("Scan a PICC to add or remove"));
}


void ledTurnOn(uint8_t LED_X) {
  if (LED_X == LED_RGB) {
    digitalWrite(LED_RED, LED_ON);
    digitalWrite(LED_GREEN, LED_ON);
    digitalWrite(LED_BLUE, LED_ON);
  }
  digitalWrite(LED_X, LED_ON);
}


void ledTurnOff(uint8_t LED_X) {
  if (LED_X == LED_RGB) {
    digitalWrite(LED_RED, LED_OFF);
    digitalWrite(LED_GREEN, LED_OFF);
    digitalWrite(LED_BLUE, LED_OFF);
  }
  digitalWrite(LED_X, LED_OFF);
}


void ledBlink(uint8_t LED_X, uint8_t times, uint8_t delaytime) {
  ledTurnOff(LED_X);
  if (LED_X == LED_RGBLOOP_TYPE1) {
    for (uint8_t i = 0; i < (times); i++) {
      digitalWrite(LED_RED, LED_ON);
      delay(delaytime);
      digitalWrite(LED_RED, LED_OFF);
      digitalWrite(LED_GREEN, LED_ON);
      delay(delaytime);
      digitalWrite(LED_GREEN, LED_OFF);
      digitalWrite(LED_BLUE, LED_ON);
      delay(delaytime);
      digitalWrite(LED_BLUE, LED_OFF);
    }
    return;
  }
  if (LED_X == LED_RGBLOOP_TYPE2) {
    for (uint8_t i = 0; i < (times); i++) {
      digitalWrite(LED_RED, LED_ON);
      delay(delaytime);
      digitalWrite(LED_RED, LED_OFF);
      delay(delaytime);
      digitalWrite(LED_GREEN, LED_ON);
      delay(delaytime);
      digitalWrite(LED_GREEN, LED_OFF);
      delay(delaytime);
      digitalWrite(LED_BLUE, LED_ON);
      delay(delaytime);
      digitalWrite(LED_BLUE, LED_OFF);
      delay(delaytime);
    }
    return;
  }
  bool LED_STATE = LED_ON;
  for (uint8_t i = 0; i < (times * 2); i++) {
    digitalWrite(LED_X, LED_STATE);
    LED_STATE = !LED_STATE;
    delay(delaytime);
  }
}


void ShowReaderDetails() {
  // Get the MFRC522 software version
  byte v = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  Serial.print(F("MFRC522 Software Version: 0x"));
  Serial.print(v, HEX);
  if (v == 0x91)
    Serial.print(F(" = v1.0"));
  else if (v == 0x92)
    Serial.print(F(" = v2.0"));
  else
    Serial.print(F(" (unknown),probably a chinese clone?"));
  Serial.println("");
  // When 0x00 or 0xFF is returned, communication probably failed
  if ((v == 0x00) || (v == 0xFF)) {
    Serial.println(F("WARNING: Communication failure, is the MFRC522 properly connected?"));
    Serial.println(F("SYSTEM HALTED: Check connections."));
    // Visualize system is halted
    ledTurnOn(LED_RED);
    while (true); // do not go further
  }
}


void touchToDoSomething() {
  ledTurnOn(LED_RED);
  if (digitalRead(TOUCHPIN) == HIGH) {
    bool buttonState = monitorTOUCHPIN(5000);
    if (buttonState == true && digitalRead(TOUCHPIN) == HIGH) {
      hintGodMode();
      while (1) {
        if (Serial.available()) {
          String str = Serial.readString();
          if (str == "A" || str == "a") {
            Serial.println("Delate all users");
            wipeEeprom();
          }
          else if (str == "B" || str == "b") {
            Serial.println("Delate admin only");
            delateAdmin();
          }
          else {
            Serial.println("Do nothing, just quit");
            ledTurnOff(LED_RED);
            return;
          }
        }
      }
    }
  }
  else {
    Serial.println("Not touched, do nothing, go on");
    ledTurnOff(LED_RED);
  }
}


void hintGodMode() {
  ledTurnOn(LED_RED);
  Serial.println("");
  Serial.println("God mode");
  Serial.println("Input 'A' to delate all users");
  Serial.println("Input 'B' to delate admin only");
  Serial.println("Input others to do nothing, just quit");
}


void wipeEeprom() {
  ledTurnOn(LED_RED);
  Serial.println(F("You have 3 seconds to cancel"));
  Serial.println(F("This will delate all users and cannot be undone"));
  bool buttonState = monitorTOUCHPIN(3000);
  if (buttonState == true && digitalRead(TOUCHPIN) == HIGH) {    // If button still be pressed, wipe EEPROM
    Serial.println(F("Starting wiping EEPROM"));
    for (uint16_t x = 0; x < EEPROM.length(); x = x + 1) {    //Loop end of EEPROM address
      if (EEPROM.read(x) == 0) {              //If EEPROM address 0
        // do nothing, already clear, go to the next address in order to save time and reduce writes to EEPROM
      }
      else {
        EEPROM.write(x, 0);       // if not write 0 to clear, it takes 3.3mS
      }
    }
    Serial.println(F("EEPROM successfully wiped"));
    Serial.println(F("Please restart to add new MasterCard"));
    while (1);
  }
  else {
    Serial.println(F("Wiping cancelled")); // Show some feedback that the wipe button did not pressed for 15 seconds
    ledTurnOff(LED_RED);
  }
  hintGodMode();
}


void delateAdmin() {
  ledTurnOn(LED_RED);
  Serial.println(F("You have 3 seconds to cancel"));
  Serial.println(F("This will delate admin and cannot be undone"));
  bool buttonState = monitorTOUCHPIN(3000);
  if (buttonState == true && digitalRead(TOUCHPIN) == HIGH) {    // If button still be pressed, wipe EEPROM
    EEPROM.write(1, 0);
    Serial.println(F("MasterCard erased from device"));
    Serial.println(F("Please restart to add new MasterCard"));
    while (1);
  }
  else {
    Serial.println(F("MasterCard erase cancelled")); // Show some feedback that the wipe button did not pressed for 15 seconds
    ledTurnOff(LED_RED);
  }
  hintGodMode();
}


uint8_t getID() {
  // Getting ready for Reading PICCs
  if ( ! mfrc522.PICC_IsNewCardPresent()) { //If a new PICC placed to RFID reader continue
    return 0;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) {   //Since a PICC placed get Serial and continue
    return 0;
  }
  // There are Mifare PICCs which have 4 byte or 7 byte UID care if you use 7 byte PICC
  // I think we should assume every PICC as they have 4 byte UID
  // Until we support 7 byte PICCs
  Serial.println(F("Scanned PICC's UID:"));
  for ( uint8_t i = 0; i < 4; i++) {  //
    readCard[i] = mfrc522.uid.uidByte[i];
    Serial.print(readCard[i], HEX);
  }
  Serial.println("");
  mfrc522.PICC_HaltA(); // Stop reading
  return 1;
}


void readID( uint8_t number ) {
  uint8_t start = (number * 4 ) + 2;    // Figure out starting position
  for ( uint8_t i = 0; i < 4; i++ ) {     // Loop 4 times to get the 4 Bytes
    storedCard[i] = EEPROM.read(start + i);   // Assign values read from EEPROM to array
  }
}


void addUser( byte a[] ) {
  if ( !findID( a ) ) {     // Before we write to the EEPROM, check to see if we have seen this card before!
    uint8_t num = EEPROM.read(0);     // Get the numer of used spaces, position 0 stores the number of ID cards
    uint8_t start = ( num * 4 ) + 6;  // Figure out where the next slot starts
    num++;                // Increment the counter by one
    EEPROM.write( 0, num );     // Write the new count to the counter
    for ( uint8_t j = 0; j < 4; j++ ) {   // Loop 4 times
      EEPROM.write( start + j, a[j] );  // Write the array values to EEPROM in the right position
    }
    ledBlink(LED_GREEN, 3, SHORTTIME);
    Serial.println(F("Succesfully added ID record to EEPROM"));
  }
  else {
    ledBlink(LED_RED, 3, LONGTIME);
    Serial.println(F("Failed! There is something wrong with ID or bad EEPROM"));
  }
}


void delateUser( byte a[] ) {
  if ( !findID( a ) ) {     // Before we delete from the EEPROM, check to see if we have this card!
    ledBlink(LED_RED, 3, LONGTIME);      // If not
    Serial.println(F("Failed! There is something wrong with ID or bad EEPROM"));
  }
  else {
    uint8_t num = EEPROM.read(0);   // Get the numer of used spaces, position 0 stores the number of ID cards
    uint8_t slot;       // Figure out the slot number of the card
    uint8_t start;      // = ( num * 4 ) + 6; // Figure out where the next slot starts
    uint8_t looping;    // The number of times the loop repeats
    uint8_t j;
    uint8_t count = EEPROM.read(0); // Read the first Byte of EEPROM that stores number of cards
    slot = findIDSLOT( a );   // Figure out the slot number of the card to delete
    start = (slot * 4) + 2;
    looping = ((num - slot) * 4);
    num--;      // Decrement the counter by one
    EEPROM.write( 0, num );   // Write the new count to the counter
    for ( j = 0; j < looping; j++ ) {         // Loop the card shift times
      EEPROM.write( start + j, EEPROM.read(start + 4 + j));   // Shift the array values to 4 places earlier in the EEPROM
    }
    for ( uint8_t k = 0; k < 4; k++ ) {         // Shifting loop
      EEPROM.write( start + j + k, 0);
    }
    ledBlink(LED_RED, 3, SHORTTIME);
    Serial.println(F("Succesfully delated ID record from EEPROM"));
  }
}


bool checkTwo ( byte a[], byte b[] ) {
  for ( uint8_t k = 0; k < 4; k++ ) {   // Loop 4 times
    if ( a[k] != b[k] ) {     // IF a != b then false, because: one fails, all fail
      return false;
    }
  }
  return true;
}


uint8_t findIDSLOT( byte find[] ) {
  uint8_t count = EEPROM.read(0);       // Read the first Byte of EEPROM that
  for ( uint8_t i = 1; i <= count; i++ ) {    // Loop once for each EEPROM entry
    readID(i);                // Read an ID from EEPROM, it is stored in storedCard[4]
    if ( checkTwo( find, storedCard ) ) {   // Check to see if the storedCard read from EEPROM
      // is the same as the find[] ID card passed
      return i;         // The slot number of the card
    }
  }
}


bool findID( byte find[] ) {
  uint8_t count = EEPROM.read(0);     // Read the first Byte of EEPROM that
  for ( uint8_t i = 1; i < count; i++ ) {    // Loop once for each EEPROM entry
    readID(i);          // Read an ID from EEPROM, it is stored in storedCard[4]
    if ( checkTwo( find, storedCard ) ) {   // Check to see if the storedCard read from EEPROM
      return true;
    }
    else {    // If not, return false
    }
  }
  return false;
}


bool isMaster( byte test[] ) {
  return checkTwo(test, masterCard);
}


bool monitorTOUCHPIN(uint32_t interval) {
  uint32_t now = (uint32_t)millis();
  while ((uint32_t)millis() - now < interval)  {
    // check on every half a second
    if (((uint32_t)millis() % 500) == 0) {
      if (digitalRead(TOUCHPIN) != HIGH)
        return false;
    }
  }
  return true;
}
