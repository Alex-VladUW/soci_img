
/**
 * Tests the GET_PICTURE_SIZE command, which returns the size
 * of the picture stored in the SD card. Will require manual verification.
 * Assumes that files were already created via the TAKE_PICTURE command
 * (Systems need to be reset since test debug with serial)
 */
void testGetPictureSize() {
  /* Checking error detection of INVALID_SLOT */
  checkInvalidSlot(GET_PICTURE_SIZE);
  /* Checking reponse of valid command */
  unsigned int returnedSizes[IMAGES_COUNT];
  for(byte i = 0x00; i < IMAGES_COUNT; i++) {
    sendCommand(GET_PICTURE_SIZE, i);
    for(int j = 0; j < MAX_RESPONSE_BYTES; j++) {
      while(Serial.available() == 0) {}
      responseBytes[j] = Serial.read();
    }
    if(responseBytes[0] != ACK ||
       responseBytes[1] != GET_PICTURE_SIZE ||
       responseBytes[2] != i) {
      Serial.print("\nFAIL: Did not give proper response when slot = "); Serial.println(i);
      Serial.print("Received: ");
      for(int j = 0; j < MAX_RESPONSE_BYTES; j++) {
        Serial.print(responseBytes[j]); Serial.print(" ");
      }
      while(true) {}
    }
    returnedSizes[i] = responseBytes[3] << 8 | responseBytes[4];
  }
  /* Success */
  Serial.println("\nSUCCESS: Passed all tests for GET_PICTURE_SIZE!");
  Serial.println("Requires manual checking: Verify the following sizes");
  for(int i = 0; i < IMAGES_COUNT; i++) {
    Serial.print(pictureNames[i]);  Serial.print(" has a total of ");
    Serial.print(returnedSizes[i]); Serial.println(" bytes");
  }
}
