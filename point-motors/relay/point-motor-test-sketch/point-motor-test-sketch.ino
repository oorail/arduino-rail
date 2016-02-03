// oorail.com - 16-channel relay board point motor testing code
//
// This code is used to test drive point motors with the Arduino
//
// visit http://youtube.com/oorail77
// This code is explained and demoed in Tech Tuesday Episode #3
// Tuesday February 2nd 2016
//
// Copyright (c) 2016 oorail.com

// The following code defines RELAYX values to pins 6,7,8,9
#define RELAY1 6
#define RELAY2 7
#define RELAY3 8
#define RELAY4 9

int count = 0; // set counter to 0

void setup() {
  // This code initializes the pins to OUTPUT
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  pinMode(RELAY3, OUTPUT);
  pinMode(RELAY4, OUTPUT);
  // This initializes the Serial Monitor for debugging
  Serial.begin(9600);
  // The 16-channel relay board defaults to LOW (on), we want it off
  // So we re-initialize the pins here to set them all to OFF
  digitalWrite(RELAY1, HIGH);
  digitalWrite(RELAY2, HIGH);
  digitalWrite(RELAY3, HIGH);
  digitalWrite(RELAY4, HIGH);
}

void loop() {
  // This main loop tracks the count, outputs some debug info to the serial port
  // This code is designed for testing point motors, it simply throws them one way and then the other
  // It will test a point motor ever 20 seconds
  count++;
  // This first pass throws the point motor one way
  Serial.print("Relay Pass #");
  Serial.println(count);
  Serial.print("Throwing Relay 1...");
  digitalWrite(RELAY1, LOW); // activate the relay on
  Serial.print("... ON ...");
  delay(1000); // wait 1 second, PECO PL-11 gets warm, may want to set this to 500
  Serial.println("... OFF...");
  digitalWrite(RELAY1, HIGH); // switch the relay to off
  delay(10000); // wait 10 seconds before moving on to the next relay

  // This second relay throws the point motor the other way
  Serial.print("Throwing Relay 2...");
  digitalWrite(RELAY2, LOW);
  Serial.print("... ON ...");
  delay(1000);
  Serial.println("... OFF...");
  digitalWrite(RELAY2, HIGH);
  delay(10000);

  // This code is used to drive a third relay for a second point motor
  Serial.print("Throwing Relay 3...");
  digitalWrite(RELAY3, LOW);
  Serial.print("... ON ...");
  delay(1000);
  Serial.println("... OFF...");
  digitalWrite(RELAY3, HIGH);
  delay(10000);

  // This code is used to drive a forth relay for a second point motor
  Serial.print("Throwing Relay 4...");
  digitalWrite(RELAY4, LOW);
  Serial.print("... ON ...");
  delay(1000);
  Serial.println("... OFF...");
  digitalWrite(RELAY4, HIGH);
  delay(10000);
  
  
}
