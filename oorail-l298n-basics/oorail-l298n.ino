/*
 *    oorail-l298n
 *    version 1.0.0
 *    
 *    Copyright (c) 2020 IdeaPier LLC, All Rights Reserved
 * 
 */

/*
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 * 
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *   
 */

/*
 *    For more information about this program visit:
 *    
 *      https://oorail.co.uk
 *      https://youtube.com/oorail
 *      
 */

/*
 *	Very simple code project to test train control with the L298N motor
 *	control board.
 */

#define OORAIL_PROJECT "oorail-l298n"
#define OORAIL_VERSION "1.0.0"
#define OORAIL_COPYRIGHT "Copyright (c) 2020 IdeaPier LLC (oorail.co.uk)"
#define OORAIL_LICENSE "GPLv3"

/*
 * Pin connections on the L298N board:
 *
 * Power Supply 12V V+ -> Vin (L298N)
 * Power Supply 12V V- -> GND (L298N)
 *
 * L298N OUT1 -> Track (+) (polarity depends on whether its UP or DOWN line)
 * L298N OUT2 -> Track (-) (opposite polarity to OUT1)
 *
 * L298N ENA -> ESP32 Pin 19 (GPIO)
 * L298N IN1 -> ESP32 Pin 18 (GPIO)
 * L298N IN2 -> ESP32 Pin 5 (GPIO)
 *
 */

const int ENA = 19;			/* ESP32 pin 19 connects to ENA on the L298N board */
const int IN1 = 18;			/* ESP32 pin 18 connects to IN1 on the L298N board */
const int IN2 = 5;			/* ESP32 pin 5 connects to IN2 on the L298N board */
const int mfreq = 30000;		/* Default Frequency to 30kHz (2kHz - 40kHz will work) */
const int mres = 10;			/* 10 bit resolution */
const int mchan = 0;			/* PWM channel 0 */

void setup() {
  Serial.begin (115200);
  oorail_banner();

  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  ledcSetup(mchan, mfreq, mres);
  ledcAttachPin(ENA, mchan);
}

void loop() {
  int mymph=0;
  ledcWrite(mchan, 0); // start stopped

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);


  Serial.println("Starting...");

  Serial.println("Initial Test Speed....");
  ledcWrite(mchan, 48);
  delay(30000);

  Serial.println("Test Speed 0...");
  ledcWrite(mchan, 64);
  delay(30000);
  Serial.println("Test speed 1...");
  ledcWrite(mchan, 128);
  delay(30000);
  Serial.println("Test speed 2...");
  ledcWrite(mchan, 256);
  delay(30000);
  Serial.println("Test speed 3...");
  ledcWrite(mchan, 384);
  delay(30000);
  Serial.println("Test speed 4...");
  ledcWrite(mchan, 512);
  delay(30000);
  Serial.println("Test speed 5...");
  ledcWrite(mchan, 640);
  delay(30000);

  ledcWrite(mchan, 640);
  delay(300);
  ledcWrite(mchan, 512);
  delay(300);
  ledcWrite(mchan, 384);
  delay(300);
  ledcWrite(mchan, 256);
  delay(300);
  ledcWrite(mchan, 128);
  delay(300);
  ledcWrite(mchan, 64);
  delay(300);
  ledcWrite(mchan, 48);
  delay(1000);
  

  // accelerate
  for (mymph=48; mymph <= 640; mymph++) {
    ledcWrite(mchan, mymph);
    delay(250);
    Serial.print("Speed ");
    Serial.print(mymph);
    Serial.println("");
  }

  // decreate to half speed

  ledcWrite(mchan, 640);

  delay(5000); // half speed for 10 seconds)

  // decrease speed

  for (mymph=640; mymph >= 48; mymph--) {
    ledcWrite(mchan, mymph);
    delay(450);
    Serial.print("Speed ");
    Serial.print(mymph);
    Serial.println("");
  }

// ledcWrite(mchan, 0);

 Serial.println("Coast test...");
 delay(60000);
  
//  delay(5000);
}

void oorail_banner() {
  Serial.println("");
  Serial.println("");
  Serial.print(OORAIL_PROJECT);
  Serial.print(", version ");
  Serial.println(OORAIL_VERSION);
  Serial.println(OORAIL_COPYRIGHT);
  Serial.println("");
  Serial.println("For additional information visit:");
  Serial.println(" https://oorail.co.uk/tech/ ");
  Serial.println("");
  Serial.print("License: ");
  Serial.println(OORAIL_LICENSE);
  Serial.println("");
}
