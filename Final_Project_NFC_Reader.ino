
#include <SPI.h>

#include <MFRC522.h>

#include <Servo.h>


#define SS_PIN 10

#define RST_PIN 9

#define SERVO_PIN 7 // Connect your servo control wire to pin 7


MFRC522 rfid(SS_PIN, RST_PIN); // RFID reader instance

Servo doorLockServo; // Servo motor instance


// Known valid RFID tag UID (replace with your tag's UID)

byte validUID[4] = {0x63, 0x63, 0x79, 0x06}; // Example UID


void setup() {

Serial.begin(9600);

SPI.begin();

rfid.PCD_Init();


doorLockServo.attach(SERVO_PIN);

doorLockServo.write(0); // Locked position


Serial.println(F("Scan RFID to unlock door."));

}


void loop() {

if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {

return;

}


Serial.println(F("Card detected:"));


// Check if the card matches the valid UID

if (isValidCard(rfid.uid.uidByte)) {

Serial.println(F("Access granted. Unlocking door..."));

doorLockServo.write(90); // Unlock position

delay(3000); // Door stays unlocked for 3 seconds

doorLockServo.write(0); // Lock again

Serial.println(F("Door locked."));

} else {

Serial.println(F("Access denied."));

}


rfid.PICC_HaltA();

rfid.PCD_StopCrypto1();

}


bool isValidCard(byte *uid) {

for (byte i = 0; i < 4; i++) {

if (uid[i] != validUID[i]) {

return false;

}

}

return true;

}
