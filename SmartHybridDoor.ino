#include <Adafruit_Fingerprint.h>
#include <LiquidCrystal.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <MFRC522.h>

// error code
#define FINGERPRINT_EXIST 0x20
#define FINGERPRINT_NOMASTER 0x21
#define SHD_CANCEL 0x22

// BUTTONS ARRAY POSITION
#define UP_BUTTON 0 // green wire
#define DOWN_BUTTON 1 // grey wire
#define BACK_BUTTON 2 // red wire
#define ENTER_BUTTON 3 // black wire

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

constexpr int buttons[] = {A1, A2, A3, A0};
String options[] = {"Enroll", "Delete", "Details", "Working"};
const int OPTION_LENGTH = (sizeof(options)/sizeof(options[0]));

int lcdCursor = 0;
int lcdCursorCharPosition = 0;
const String lcdCursorChar = "*";
// function init
void printError(uint8_t errorCode);
uint8_t getFingerId();
uint8_t fingerEnroll(uint8_t socketId);
int checkButtons(uint16_t time = 1);
void selectOption();
uint8_t saveName(uint8_t socketId);

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
	pinMode(buttons[DOWN_BUTTON],INPUT_PULLUP);
	pinMode(buttons[BACK_BUTTON],INPUT_PULLUP);
	pinMode(buttons[UP_BUTTON],INPUT_PULLUP);
	pinMode(buttons[ENTER_BUTTON],INPUT_PULLUP);
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
		int counter = 0;
		if(true){
			for(int i = lcdCursor; counter < 2; i++){
				lcd.print(options[i]);
				lcd.setCursor(0,1);
				counter++;
			}
			lcd.setCursor(15,lcdCursorCharPosition);
			lcd.print(lcdCursorChar);
			int buttonStatus = checkButtons(1000);
			delay(300);

			switch (buttonStatus){
				case buttons[UP_BUTTON]:
					if(lcdCursorCharPosition == 0)
						lcdCursor--;
					else if(lcdCursorCharPosition == 1)
						lcdCursorCharPosition = 0;
					if(lcdCursor <= 0)
						lcdCursor = 0;
					break;
				case buttons[DOWN_BUTTON]:
					if(lcdCursorCharPosition == 1)
						lcdCursor++;
					else if(lcdCursorCharPosition == 0)
						lcdCursorCharPosition = 1;
					if(lcdCursor >= OPTION_LENGTH-1)
						lcdCursor = OPTION_LENGTH-2;
					break;
				case buttons[ENTER_BUTTON]:
					selectOption();
					break;
				case buttons[BACK_BUTTON]:
					lcdCursor = 0;
					lcdCursorCharPosition = 0;
					break;
			}
		}
	}
}

void selectOption(){
	if(lcdCursor == 0 && lcdCursorCharPosition == 0){ // Enroll
		while(true){
			lcd.clear();
			finger.getTemplateCount();
			int registerId = finger.templateCount+1;
			lcd.print("Register ID: ");
			lcd.print(registerId);
			delay(1000);
			int errorCode = fingerEnroll(registerId);
			if(errorCode == FINGERPRINT_OK){
				saveName(registerId);
				return;
			}else{
				printError(errorCode);
				if(errorCode == SHD_CANCEL)
					return;
			}
		}
	}else if((lcdCursor == 0 && lcdCursorCharPosition == 1) || (lcdCursor == 1 && lcdCursorCharPosition == 0)){ // Delete
		lcd.clear();
		lcd.print("Delete");
	}else if((lcdCursor == 1 && lcdCursorCharPosition == 1) || (lcdCursor == 2 && lcdCursorCharPosition == 0)){ // Details
		lcd.clear();
		lcd.print("Details");
	}else if((lcdCursor == 2 && lcdCursorCharPosition == 1)){ // Working
		lcd.clear();
		lcd.print("Working");
	}else{
		lcd.clear();
		lcd.print("Cursor: ");
		lcd.print(lcdCursor);
		lcd.setCursor(0,1);
		lcd.print("Char: ");
		lcd.print(lcdCursorCharPosition);
	}
	delay(2500);
}

uint8_t saveName(uint8_t socketId){
	return;
}

int checkButtons(uint16_t time = 1){
	Serial.print("start    ");
	Serial.println(time);

	for(uint16_t i = 0; i <= time; i++){
		Serial.print("Counter: ");
		Serial.println(i);
		delay(1);
		Serial.print("UP: "); Serial.println(digitalRead(buttons[UP_BUTTON]));
		Serial.print("DOWN: "); Serial.println(digitalRead(buttons[DOWN_BUTTON]));
		Serial.print("BACK: "); Serial.println(digitalRead(buttons[BACK_BUTTON]));
		Serial.print("ENTER: "); Serial.println(digitalRead(buttons[ENTER_BUTTON]));
		delay(1);
		if(digitalRead(buttons[UP_BUTTON]))
			return buttons[UP_BUTTON];
		else if(digitalRead(buttons[DOWN_BUTTON]))
			return buttons[DOWN_BUTTON];
		else if(digitalRead(buttons[BACK_BUTTON]))
			return buttons[BACK_BUTTON];
		else if(digitalRead(buttons[ENTER_BUTTON]))
			return buttons[ENTER_BUTTON];
	}
	return false;
}

uint8_t fingerEnroll(uint8_t socketId){
	p = 2; 
	lcd.clear();
	lcd.print("Put your finger"); 
	while (p != FINGERPRINT_OK) {
    p = finger.getImage();
		if(checkButtons() == buttons[BACK_BUTTON])
			return SHD_CANCEL;
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
			case SHD_CANCEL:
				lcd.print("Cancel");
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

