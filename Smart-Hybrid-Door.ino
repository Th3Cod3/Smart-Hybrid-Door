#include <Adafruit_Fingerprint.h>
#include <LiquidCrystal.h>
#include <SoftwareSerial.h>
#include <Stepper.h>
#include <SPI.h>
#include <MFRC522.h>

// ERROR CODE
#define FINGERPRINT_EXIST 0x20
#define FINGERPRINT_NOMASTER 0x21
#define SHD_CANCEL 0x22

// BUTTONS ARRAY POSITION
#define UP_BUTTON 0 // green wire
#define DOWN_BUTTON 1 // grey wire
#define BACK_BUTTON 2 // red wire
#define ENTER_BUTTON 3 // black wire

// MOTOR
#define MOTOR_PIN4 8
#define MOTOR_PIN3 7
#define MOTOR_PIN2 6
#define MOTOR_PIN1 A7
#define GEAR_STEPS 120
#define MOTOR_DELAY 2

// RFID SPI PINS
#define SS_PIN 10
#define RST_PIN 9

// LCD PINS
#define ENABLE A1
#define READ_SET A0
#define DATA_4 A2
#define DATA_5 A3
#define DATA_6 A4
#define DATA_7 A5


constexpr int buttons[] = {4, 3, 2, 5};
String options[] = {"Enroll", "Delete", "Details", "Working", "Factory restore"};
const int OPTION_LENGTH = (sizeof(options)/sizeof(options[0]));

int lcdCursor = 0;
int lcdCursorCharPosition = 0;
const String lcdCursorChar = "*";

// function init
void printError(uint8_t errorCode);
uint8_t getFingerId();
uint8_t fingerEnroll(uint8_t socketId);
int checkButtons(uint16_t time = 1);
uint8_t saveName(uint8_t socketId);
void selectOption();
void openDoor();
void closeDoor();
void masterRegister();


// Module objects564
LiquidCrystal lcd(READ_SET, ENABLE, DATA_4, DATA_5, DATA_6, DATA_7);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial);
MFRC522 mfrc522(SS_PIN, RST_PIN);

// global variable
uint8_t id;
uint8_t statusCode = -1;

void setup() {
	pinMode(buttons[DOWN_BUTTON],INPUT_PULLUP);
	pinMode(buttons[BACK_BUTTON],INPUT_PULLUP);
	pinMode(buttons[UP_BUTTON],INPUT_PULLUP);
	pinMode(buttons[ENTER_BUTTON],INPUT_PULLUP);
	pinMode(MOTOR_PIN1,OUTPUT);
	pinMode(MOTOR_PIN2,OUTPUT);
	pinMode(MOTOR_PIN3,OUTPUT);
	pinMode(MOTOR_PIN4,OUTPUT);
	lcd.begin(16,2);	
	closeDoor();
	lcd.clear();
	lcd.print("Starting...");
	SPI.begin();	// Init SPI bus
	mfrc522.PCD_Init();	// Init MFRC522
	delay(500);
	finger.begin(57600);
	lcd.clear();
	lcd.print("Finger module");
	while(!finger.verifyPassword()){
		lcd.clear();
		lcd.print("Check password");
		lcd.setCursor(0,1);
		lcd.print("Connect module");
		delay(1000);
	}
	lcd.clear();
	lcd.print("Password OK.");
	finger.getTemplateCount();

	delay(500);

}
void loop() {
	lcd.clear();
	int counter = 0;
	if(true){
		if(finger.templateCount == 0)
			masterRegister();
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

void selectOption(){
	if(lcdCursor == 0 && lcdCursorCharPosition == 0){ // Enroll
		uint8_t errorCode = 0;
		while(true){
			lcd.clear();
			lcd.print("Need MASTER"); 
			lcd.setCursor(0,1);
			lcd.print("Finger");
			delay(2000);
			errorCode = getFingerId();
			if(errorCode == SHD_CANCEL)
						return;
			if(finger.fingerID == 1){
				lcd.clear();
				finger.getTemplateCount();
				uint8_t registerId = finger.templateCount+1;
				lcd.print("Register ID: ");
				lcd.print(registerId);
				delay(1000);
				errorCode = fingerEnroll(registerId);
				if(errorCode == FINGERPRINT_OK){
					saveName(registerId);
					return;
				}else{
					printError(errorCode);
					if(errorCode == SHD_CANCEL)
						return;
				}
			}else
				printError(FINGERPRINT_NOMASTER);

		}
	}else if((lcdCursor == 0 && lcdCursorCharPosition == 1) || (lcdCursor == 1 && lcdCursorCharPosition == 0)){ // Delete
		lcd.clear();
		lcd.print("Delete");
	}else if((lcdCursor == 1 && lcdCursorCharPosition == 1) || (lcdCursor == 2 && lcdCursorCharPosition == 0)){ // Details
		lcd.clear();
		lcd.print("Ver. 0.1.0");
		lcd.setCursor(0,1);
		lcd.print("Total Reg. ");
		lcd.print(finger.templateCount);
		while(checkButtons() != buttons[BACK_BUTTON])
			delay(200);
		return;
	}else if(lcdCursor == 2 && lcdCursorCharPosition == 1 || (lcdCursor == 3 && lcdCursorCharPosition == 0)){ // Working
		while(true){
			lcd.clear();
			lcd.print("Scanning...");
			statusCode = getFingerId();
			if(statusCode == FINGERPRINT_OK){
				openDoor();
				delay(1000);
				lcd.clear();
				lcd.print("Finger ID: ");
				lcd.print(finger.fingerID);
				delay(5000);
				closeDoor();
				delay(1000);
			}else if(statusCode == SHD_CANCEL){
				return;
			}else{
				printError(statusCode);
			}
		}
	}else if(lcdCursor == 3 && lcdCursorCharPosition == 1 || (lcdCursor == 4 && lcdCursorCharPosition == 0)){ //Factory restore
		lcd.clear();
		lcd.print("Need MASTER");
		lcd.setCursor(0,1);
		lcd.print("Finger");
		delay(2000);
		uint8_t errorCode = 0;
		errorCode = getFingerId();
		if(errorCode == SHD_CANCEL)
			return;
		if(finger.fingerID == 1){
			lcd.clear();
			lcd.print("Restoring");
			finger.emptyDatabase();
			finger.getTemplateCount();
		}else{
			lcd.clear();
			lcd.print("No master ID");
		}
		
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
	return 12;
}

int checkButtons(uint16_t time = 1){
	for(uint16_t i = 0; i <= time; i++){
		if(digitalRead(buttons[UP_BUTTON]))
			return buttons[UP_BUTTON];
		else if(digitalRead(buttons[DOWN_BUTTON]))
			return buttons[DOWN_BUTTON];
		else if(digitalRead(buttons[BACK_BUTTON]))
			return buttons[BACK_BUTTON];
		else if(digitalRead(buttons[ENTER_BUTTON]))
			return buttons[ENTER_BUTTON];
		delay(1);
	}
	return false;
}

uint8_t fingerEnroll(uint8_t socketId){
	statusCode = 2; 
	lcd.clear();
	lcd.print("Put your finger"); 
	while (statusCode != FINGERPRINT_OK) {
    statusCode = finger.getImage();
		if(checkButtons() == buttons[BACK_BUTTON])
			return SHD_CANCEL;
  }
	statusCode = finger.image2Tz(1);
	lcd.clear();
	lcd.print("Remove finger"); 
	if(statusCode != FINGERPRINT_OK)
		return statusCode;
	statusCode = finger.fingerFastSearch();
	if(statusCode == FINGERPRINT_OK)
		return FINGERPRINT_EXIST;
	delay(2000);
	while (statusCode != FINGERPRINT_NOFINGER) {
    statusCode = finger.getImage();
  }
  statusCode = 2;
	lcd.clear();
	lcd.print("Put finger again"); 
  while (statusCode != FINGERPRINT_OK) {
    statusCode = finger.getImage();
		if(checkButtons() == buttons[BACK_BUTTON])
			return SHD_CANCEL;
  }
	statusCode = finger.image2Tz(2);
	if(statusCode != FINGERPRINT_OK)
		return statusCode;
	lcd.clear();
	lcd.print("Proccessing..."); 
	delay(1000);
	statusCode = finger.createModel();
	if(statusCode != FINGERPRINT_OK)
		return statusCode;
	lcd.clear();
	lcd.print("Prints matched!"); 
	delay(1000);
	statusCode = finger.storeModel(socketId);
	if(statusCode != FINGERPRINT_OK)
		return statusCode;
	lcd.clear();
	lcd.print("Stored!"); 
	finger.getTemplateCount();
	delay(1000);
	
	return statusCode;
}

uint8_t getFingerId() {
  statusCode = 2; 
	lcd.clear();
	lcd.print("Put your finger"); 
	while (statusCode != FINGERPRINT_OK) {
    statusCode = finger.getImage();
		if(checkButtons(300) == buttons[BACK_BUTTON])
			return SHD_CANCEL;
  }
	statusCode = finger.image2Tz(1);
	lcd.clear();
	lcd.print("Remove finger"); 
	if(statusCode != FINGERPRINT_OK)
		return statusCode;
	delay(2000);

  statusCode = finger.fingerFastSearch();
  if (statusCode != FINGERPRINT_OK)  return statusCode;
  return statusCode; 
}

void masterRegister(){
	lcd.clear();
	lcd.print("Register Master");
	lcd.setCursor(0,1);
	lcd.print("Press ENTER");
	while(checkButtons() != buttons[ENTER_BUTTON]);
	return printError(fingerEnroll(1));
}

void closeDoor(){
	lcd.clear();
	lcd.print("CLOSING DOOR");
	for(int i = 0; i < GEAR_STEPS+40; i++)
	{
		digitalWrite(MOTOR_PIN1,HIGH);
		digitalWrite(MOTOR_PIN2,HIGH);
		digitalWrite(MOTOR_PIN3,LOW);
		digitalWrite(MOTOR_PIN4,LOW);
		delay(MOTOR_DELAY);
		digitalWrite(MOTOR_PIN1,LOW);
		digitalWrite(MOTOR_PIN2,HIGH);
		digitalWrite(MOTOR_PIN3,HIGH);
		digitalWrite(MOTOR_PIN4,LOW);
		delay(MOTOR_DELAY);
		digitalWrite(MOTOR_PIN1,LOW);
		digitalWrite(MOTOR_PIN2,LOW);
		digitalWrite(MOTOR_PIN3,HIGH);
		digitalWrite(MOTOR_PIN4,HIGH);
		delay(MOTOR_DELAY);
		digitalWrite(MOTOR_PIN1,HIGH);
		digitalWrite(MOTOR_PIN2,LOW);
		digitalWrite(MOTOR_PIN3,LOW);
		digitalWrite(MOTOR_PIN4,HIGH);
		delay(MOTOR_DELAY);
	}
	digitalWrite(MOTOR_PIN1,LOW);
	digitalWrite(MOTOR_PIN2,LOW);
	digitalWrite(MOTOR_PIN3,LOW);
	digitalWrite(MOTOR_PIN4,LOW);
	lcd.clear();
	lcd.print("CLOSE DOOR");
}

void openDoor(){
	lcd.clear();
	lcd.print("OPENING DOOR");
	for(int i = 0; i < GEAR_STEPS; i++)
	{
		digitalWrite(MOTOR_PIN1,HIGH);
		digitalWrite(MOTOR_PIN2,HIGH);
		digitalWrite(MOTOR_PIN3,LOW);
		digitalWrite(MOTOR_PIN4,LOW);
		delay(MOTOR_DELAY);
		digitalWrite(MOTOR_PIN1,HIGH);
		digitalWrite(MOTOR_PIN2,LOW);
		digitalWrite(MOTOR_PIN3,LOW);
		digitalWrite(MOTOR_PIN4,HIGH);
		delay(MOTOR_DELAY);
		digitalWrite(MOTOR_PIN1,LOW);
		digitalWrite(MOTOR_PIN2,LOW);
		digitalWrite(MOTOR_PIN3,HIGH);
		digitalWrite(MOTOR_PIN4,HIGH);
		delay(MOTOR_DELAY);
		digitalWrite(MOTOR_PIN1,LOW);
		digitalWrite(MOTOR_PIN2,HIGH);
		digitalWrite(MOTOR_PIN3,HIGH);
		digitalWrite(MOTOR_PIN4,LOW);
		delay(MOTOR_DELAY);
	}
	digitalWrite(MOTOR_PIN1,LOW);
	digitalWrite(MOTOR_PIN2,LOW);
	digitalWrite(MOTOR_PIN3,LOW);
	digitalWrite(MOTOR_PIN4,LOW);
	lcd.clear();
	lcd.print("OPEN DOOR");
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

