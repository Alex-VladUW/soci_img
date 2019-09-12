/**
 * Tests the performance of the IMG software from the point
 * of view of an external device. (Built for v1.0.0)
 * Intended to be uploaded to an arduino Uno or similar device
 * 
 */
#include "v1.0.0-performance.h"

/* Unused port used for the seed of random generation */
static const unsigned short UNUSED_PORT = A0;

void setup() {
  Serial.begin(115200);
  randomSeed(analogRead(UNUSED_PORT));
  testCommand(TAKE_PICTURE);
  printResults(printInfoArray);
}

/* Tests the 'command' 'TEST_TIMES' number of times and
 * stores the time taken each time in 'timeStorage'
 ********************************************************
 * Function is purposely written in a verbose fashion
 * without function calls to get more accurate timings
 */
void testCommand(byte command) {
  if(command == GET_THUMBNAIL || command == GET_PICTURE) {
    header = "time(ms)\tslotSent|slotReceived";
    fetchImageSizes(command - 0x02);
    for(int i = 0; i < TEST_TIMES; i++) {
      byte toSend[] = {command, (byte)random(IMAGES_COUNT)};
      long startTime = millis();
      Serial.write(toSend, sizeof(toSend));
      while(!Serial.available()) {}
      byte receivedResponse = Serial.read();
      while(!Serial.available()) {}
      byte slot = Serial.read();
      if(receivedResponse != ACK) {
        timeStorage[i] = 0;
        continue;
      }
      /* Retrieving data */
      unsigned int totalSize = storedSize[toSend[1]];
      unsigned int packages = totalSize / EXTERNAL_PACKAGE_SIZE;
      unsigned int remainingBytes = totalSize % EXTERNAL_PACKAGE_SIZE;
      for(int i = 0; i < packages; i++) {
        Serial.write(ACK);
        while(!Serial.available()) {}
        for(int j = 0; j < EXTERNAL_PACKAGE_SIZE; j++) {
          Serial.read();
        }
      }
      Serial.write(ACK);
      for(int i = 0; i < remainingBytes; i++) {
        Serial.read();
      }
      long endTime = millis();
      testInfo[i] = toSend[1] << 4 | slot;
      timeStorage[i] = endTime - startTime;
    }
  }
  else if(command == TAKE_PICTURE) {
    header = "time(ms)\tslotSent|slotReceived";
    for(int i = 0; i < TEST_TIMES; i++) {
      byte toSend[] = {command, (byte)random(IMAGES_COUNT)};
      long startTime = millis();
      Serial.write(toSend, sizeof(toSend));
      while(!Serial.available()) {}
      byte receivedResponse = Serial.read();
      while(!Serial.available()) {}
      byte slot = Serial.read();
      long endTime = millis();
      testInfo[i] = toSend[1] << 4 | slot;
      timeStorage[i] = 0;
      if(receivedResponse == ACK) {
        timeStorage[i] = endTime - startTime;
      }
    }
  }
}

/* Fills the 'storedSize' with sizes based on 'command'
 * which must be:
 * 0x01 (Thumbnail Size) or 0x02 (Picture Size)
 */
void fetchImageSizes(byte command) {
  if((command != GET_THUMBNAIL_SIZE) && (command != GET_PICTURE_SIZE)) {
    Serial.print("ERROR! 'command' = "); Serial.println(command, HEX);
    while(1) {}
  }
  const unsigned short REPEAT_TIMES = 5;
  unsigned int tempSizeStorage[REPEAT_TIMES];
  for(int i = 0; i < IMAGES_COUNT; i++) {
    for(int j = 0; j < REPEAT_TIMES; j++) {
      byte toSend[] = {command, (byte)i};
      Serial.write(toSend, sizeof(toSend));
      while(!Serial.available()) {}
      delay(5);
      if(Serial.read() != ACK) {
        Serial.println("FAILED on ACK");
        while(1) {}
      }
      tempSizeStorage[j] = (Serial.read() << 8) | Serial.read();
    }
    // Checking if the result is consistent
    unsigned int firstSize = tempSizeStorage[0];
    for(int j = 1; j < REPEAT_TIMES; j++) {
      if(tempSizeStorage[j] != firstSize) {
        Serial.println("ERROR: Sizes mismatch");
        while(1) {}
      }
    }
    storedSize[i] = firstSize;
  }
}

/* Print out all stored time in 'timeStorage' and 'testInfo', one test per line */
void printResults(bool enableInfo) {
  Serial.println(header);
  for(int i = 0; i < TEST_TIMES; i++) {
    Serial.print(timeStorage[i], DEC);
    if(enableInfo) {
      Serial.print("\t"); Serial.print(testInfo[i]);
    }
    Serial.println();
  }
}

/* Not used */
void loop() {}
