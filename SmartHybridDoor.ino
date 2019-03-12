
#include <Adafruit_Fingerprint.h>
#include <LiquidCrystal.h>
#include <SoftwareSerial.h>
#define FINGERPRINT_EXIST 0x20
#define FINGERPRINT_NOMASTER 0x21

const int
	// buttons
	ENROLL_SWITCH = A1,
	VERIFY_SWITCH = A2,
	DELETE_SWITCH = A3,
	BACKDOOR_SWITCH = A0,
	// SPI 
	PIN_SS = A4,
	PIN_RST = 10,
	//LCD
	ENABLE = 9,
	READ_SET = 8,
	DATA_4 = 7,
	DATA_5 = 6,
	DATA_6 = 5,
	DATA_7 = 4;
boolean 
	// status
	enrollSwitchStatus = false, 
	verifySwitchStatus = false, 
	deleteSwitchStatus = false,
	backdoorSwitchStatus = false;
LiquidCrystal lcd(READ_SET, ENABLE, DATA_4, DATA_5, DATA_6, DATA_7);
SoftwareSerial mySerial(2, 3);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

uint8_t id;
uint8_t p = -1;
void setup() {
	pinMode(VERIFY_SWITCH,INPUT);
	pinMode(DELETE_SWITCH,INPUT);
	pinMode(ENROLL_SWITCH,INPUT);
	pinMode(BACKDOOR_SWITCH,INPUT);
	Serial.begin(9600);	// Init Serial
	SPI.begin();	// Init SPI bus
	mfrc522.PCD_Init();	// Init MFRC522
	lcd.begin(16,2);	
	lcd.print("Starting...");
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
	while(finger.templateCount == 0 || ENROLL_SWITCH == LOW){
		lcd.clear();
		lcd.print("    Welcome");
		lcd.setCursor(0,1);
		lcd.print("SmartHybridDoor");
		if(checkOptions(2000))
			break;
		lcd.clear();
		lcd.print("For the first");
		lcd.setCursor(0,1);
		lcd.print("time you need");
		if(checkOptions(2000))
			break;
		lcd.clear();
		lcd.print("to register the");
		lcd.setCursor(0,1);
		lcd.print("MASTER finger.");
		if(checkOptions(2000))
			break;
		lcd.clear();
		lcd.print("Press 8 for");
		lcd.setCursor(0,1);
		lcd.print("enroll");
		if(checkOptions(4000))
			break;
	}
	while(!checkOptions(2000)){
		lcd.clear();
		lcd.print("8 FOR ENROLL");
		lcd.setCursor(0,1);
		lcd.print("4 TO VERIFY");
		if(checkOptions(2000))
			break;
		lcd.clear();
		lcd.print("1 DELETE ALL");
		lcd.setCursor(0,1);
		lcd.print("TOTAL FINGER ");
		lcd.print(finger.templateCount);
		if(checkOptions(2000))
			break;
	}

	if(enrollSwitchStatus){
		if(finger.templateCount == 0)
			printError(fingerEnroll(1));
		else{
			lcd.clear();
			lcd.print("Need MASTER"); 
			lcd.setCursor(0,1);
			lcd.print("Finger");
			delay(2000);
			getFingerId();
			if(finger.fingerID == 1)
				printError(fingerEnroll(finger.templateCount+1));
			else
				printError(FINGERPRINT_NOMASTER);
		}
	}else if(backdoorSwitchStatus){
		printError(fingerEnroll(1));
	}else if(deleteSwitchStatus){
		lcd.clear();
		lcd.print("Need MASTER"); 
		lcd.setCursor(0,1);
		lcd.print("Finger");
		delay(2000);
		getFingerId();
		if(finger.fingerID == 1){
			finger.emptyDatabase();
			finger.getTemplateCount();
		}else
			printError(FINGERPRINT_NOMASTER);
	}else if(verifySwitchStatus){
		int response = getFingerId();
		if(response == FINGERPRINT_OK){
			lcd.clear();
			lcd.print("Unlocked"); 
			lcd.setCursor(0,1);
			lcd.print("ID: "); lcd.print(finger.fingerID);
			delay(2000);
		}else
			printError(response);
	}

	lcd.clear();
	lcd.print("Finish!");
	delay(1000);
}

boolean checkOptions(uint16_t time){
	for(int i = 0; i <= time; i++){
		if (digitalRead(VERIFY_SWITCH) || digitalRead(DELETE_SWITCH) || digitalRead(ENROLL_SWITCH) || digitalRead(BACKDOOR_SWITCH)){
			verifySwitchStatus = digitalRead(VERIFY_SWITCH);
			deleteSwitchStatus = digitalRead(DELETE_SWITCH);
			enrollSwitchStatus = digitalRead(ENROLL_SWITCH);
			backdoorSwitchStatus = digitalRead(BACKDOOR_SWITCH);
			return true;
		}
		delay(1);
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
	return FINGERPRINT_OK;
}
