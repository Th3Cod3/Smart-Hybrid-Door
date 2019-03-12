#include <Adafruit_Fingerprint.h>
#include <LiquidCrystal.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <MFRC522.h>

// error code
#define FINGERPRINT_EXIST 0x20
#define FINGERPRINT_NOMASTER 0x21

// BUTTONS
#define UP_BUTTON 19 // green wire
#define DOWN_BUTTON 20 // grey wire
#define BACK_BUTTON 21 // red wire
#define ENTER_BUTTON 18 // black wire

// RFID SPI PINS
#define SS_PIN 10
#define RST_PIN A4

// LCD PINS
#define ENABLE 9
#define READ_SET 8
#define DATA_4 7
#define DATA_5 6
#define DATA_6 5
#define DATA_7 4

int lcdCursor = 0;
int lcdCursorCharPosition = 0;
const String lcdCursorChar = "*";
// function init
void printError(uint8_t errorCode);
uint8_t getFingerId();
uint8_t fingerEnroll(uint8_t socketId);
int checkButton(uint16_t time);

// Module objects
LiquidCrystal lcd(READ_SET, ENABLE, DATA_4, DATA_5, DATA_6, DATA_7);
SoftwareSerial mySerial(2, 3);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
MFRC522 mfrc522(SS_PIN, RST_PIN); 

// global varaible
uint8_t id;
uint8_t p = -1;

void setup() {
	Serial.begin(9600);
	lcd.begin(16,2);	
	lcd.print("Starting...");
	pinMode(DOWN_BUTTON,INPUT);
	pinMode(BACK_BUTTON,INPUT);
	pinMode(UP_BUTTON,INPUT);
	pinMode(ENTER_BUTTON,INPUT);
	SPI.begin();	// Init SPI bus
	mfrc522.PCD_Init();	// Init MFRC522
	delay(500);
	finger.begin(57600);

	lcd.clear();
	while(!finger.verifyPassword()){
		lcd.print("Check password");
		lcd.setCursor(0,1);
		lcd.print("Connect module");
	}
	lcd.print("Password OK.");
	finger.getTemplateCount();
	delay(500);
}
void loop() {
	// register master
		// while(finger.templateCount == 0){
		// 	lcd.clear();
		// 	lcd.print("    Welcome");
		// 	lcd.setCursor(0,1);
		// 	lcd.print("SmartHybridDoor");
		// 	if(checkOptions(2000))
		// 		break;
		// 	lcd.clear();
		// 	lcd.print("For the first");
		// 	lcd.setCursor(0,1);
		// 	lcd.print("time you need");
		// 	if(checkOptions(2000))
		// 		break;
		// 	lcd.clear();
		// 	lcd.print("to register the");
		// 	lcd.setCursor(0,1);
		// 	lcd.print("MASTER finger.");
		// 	if(checkOptions(2000))
		// 		break;
		// 	lcd.clear();
		// 	lcd.print("Press 8 for");
		// 	lcd.setCursor(0,1);
		// 	lcd.print("enroll");
		// 	if(checkOptions(4000))
		// 		break;
	// }

	while(true){
		lcd.clear();
		String options[] = {"Enroll", "Delete", "Details", "Working"};
		int counter = 0;
		if(true){
			for(int i = lcdCursor; counter < 2; i++){
				lcd.print(options[i]);
				lcd.setCursor(0,1);
				counter++;
			}
			lcd.setCursor(15,lcdCursorCharPosition);
			lcd.print(lcdCursorChar);
			int buttonStatus = checkButton(100);

			switch (buttonStatus){
				case UP_BUTTON:
					if(lcdCursorCharPosition == 0)
						lcdCursor--;
					else if(lcdCursorCharPosition == 1)
						lcdCursorCharPosition = 0;
					break;
				case DOWN_BUTTON:
					if(lcdCursorCharPosition == 1)
						lcdCursor++;
					else if(lcdCursorCharPosition == 0)
						lcdCursorCharPosition = 1;
					break;
				case ENTER_BUTTON:
					/* code */
					break;
				case BACK_BUTTON:
					/* code */
					break;
			}
		}
	}
}

int checkButton(uint16_t time){
	Serial.print("start    ");
	Serial.println(time);

	for(uint16_t i = 0; i <= time; i++){
		Serial.print("Counter: ");
		Serial.println(i);
		delay(1);
		if(digitalRead(UP_BUTTON))
			return UP_BUTTON;
		else if(digitalRead(DOWN_BUTTON))
			return DOWN_BUTTON;
		else if(digitalRead(BACK_BUTTON))
			return BACK_BUTTON;
		else if(digitalRead(ENTER_BUTTON))
			return ENTER_BUTTON;
	}
	return false;
}

uint8_t fingerEnroll(uint8_t socketId){
	p = 2; 
	lcd.clear();
	lcd.print("Put your finger"); 
	while (p != FINGERPRINT_OK) {
    p = finger.getImage();
  }
	p = finger.image2Tz(1);
	lcd.clear();
	lcd.print("Remove finger"); 
	if(p != FINGERPRINT_OK)
		return p;
	p = finger.fingerFastSearch();
	if(p == FINGERPRINT_OK)
		return FINGERPRINT_EXIST;
	delay(2000);
	while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  p = 2;
	lcd.clear();
	lcd.print("Put finger again"); 
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
  }
	p = finger.image2Tz(2);
	if(p != FINGERPRINT_OK)
		return p;
	lcd.clear();
	lcd.print("Proccessing..."); 
	delay(1000);
	p = finger.createModel();
	if(p != FINGERPRINT_OK)
		return p;
	lcd.clear();
	lcd.print("Prints matched!"); 
	delay(1000);
	p = finger.storeModel(socketId);
	if(p != FINGERPRINT_OK)
		return p;
	lcd.clear();
	lcd.print("Stored!"); 
	finger.getTemplateCount();
	delay(1000);
	
	return p;
}

uint8_t getFingerId() {
  p = 2; 
	lcd.clear();
	lcd.print("Put your finger"); 
	while (p != FINGERPRINT_OK) {
    p = finger.getImage();
  }
	p = finger.image2Tz(1);
	lcd.clear();
	lcd.print("Remove finger"); 
	if(p != FINGERPRINT_OK)
		return p;
	delay(2000);

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return p;
  return p; 
}

void printError(uint8_t errorCode){
	if(errorCode != FINGERPRINT_OK){
		lcd.clear();
		switch (errorCode){
			case FINGERPRINT_NOTFOUND:
				lcd.print("Not registered");
				break;
			case FINGERPRINT_NOMATCH:
				lcd.print("No match");
				break;
			case FINGERPRINT_NOFINGER:
				lcd.print("No finger");
				break;
			case FINGERPRINT_EXIST:
				lcd.print("Already exist");
				break;
			case FINGERPRINT_NOMASTER:
				lcd.print("No MASTER ID");
				break;
			default:
				lcd.print("Something wrong.");
				lcd.setCursor(0,1);
				lcd.print("Error: ");
				if(errorCode > 16)
					lcd.print(0);
				lcd.print(errorCode, HEX);
				break;
		}
		delay(2000);
		lcd.clear();
		return;
	}
	return;
}

