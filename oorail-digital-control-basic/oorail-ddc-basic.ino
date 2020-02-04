/*
 *    oorail-ddc-basic
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
 *      https://oorail.co.uk/tech
 *      https://youtube.com/oorail
 *      
 */

/* NOTE: This project is an early (basic) version of our oorail-system track module
 *       The code in this repository is not regularly maintained and is experimental.
 *       It is primarily here for educational purposes so its easier to understand
 *       some of the fundamental concepts in our Digital DC Controller (oorail-system).
 *
 *       Please check the oorail-system repository for the latest and better release of this code
 */

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>

#define OORAIL_PROJECT "oorail-ddc-basic"
#define OORAIL_VERSION "1.0.0"
#define OORAIL_COPYRIGHT "Copyright (c) 2020 IdeaPier LLC (oorail.co.uk)"
#define OORAIL_LICENSE "GPLv3"
#define OORAIL_PORT 80

#define OORAIL_DEFAULT_MAXSPEED 984
#define OORAIL_DEFAULT_SHUNT 56
#define OORAIL_DEFAULT_SPEEDSTEP 8
#define OORAIL_DEFAULT_BRAKE 32

typedef struct oorail_speed_t
{
  unsigned int brake;
  unsigned int stopped;
  unsigned int current_speed;
  unsigned int target_speed;
  unsigned int coast_speed;
  unsigned int max_speed;
};

const int WIFI_LED = 2;  /* Built-In WiFi LED pin */
const int ENA = 19;      /* ENA pin for control of track A */
const int IN1 = 18;      /* IN1 for track A */
const int IN2 = 5;       /* IN2 for track A */
const int mfreq = 28000; /* PWM  Frequency */
const int mres = 10;     /* resolution 10-bit */
const int mchan = 0;     /* channel */

/* WiFi credentials */
const char* ssid = "REPLACEME-WIFI-SSID";
const char* password = "REPLACEME-WIFI-SSID";

/* speed in mph */
int oorail_mph;

/* track A state */
oorail_speed_t  oorail_speed;

/* Create web server instance */
WebServer server(OORAIL_PORT);

/* HTTP Server */
void oorail_web_init() {
  server.on("/", HTTP_GET, oorail_web_handler_root);
  server.on("/speed/current", HTTP_GET, oorail_web_askspeed);
  server.on("/speed/up", HTTP_GET, oorail_web_accel);
  server.on("/speed/down", HTTP_GET, oorail_web_decel);
  server.on("/speed/stop", HTTP_GET, oorail_web_stop);
  server.on("/coast", HTTP_GET, oorail_web_coast);
  server.on("/shunt", HTTP_GET, oorail_web_shunt);
  server.on("/stop", HTTP_GET, oorail_web_estop);
  server.on("/brake", HTTP_GET, oorail_web_brake);
  server.on("/brake/on", HTTP_GET, oorail_web_brakeon);
  server.on("/brake/off", HTTP_GET, oorail_web_brakeoff);
  server.on("/debug", HTTP_GET, oorail_web_debug);
  server.on("/ping", []() {
    server.send(200, "text/plain", "ping OK");
  });
  server.onNotFound(oorail_web_err_notfound);
  server.begin();
  Serial.println(" * HTTP server initialized ");
}

void oorail_web_debug() {
  char response[512];
  digitalWrite(WIFI_LED, 1);
  sprintf(response, "\r\nbrake = %d\r\nstopped = %d\r\ncurrent = %d\r\ntarget = %d\r\ncoast = %d\r\nmax = %d\r\n",
    oorail_speed.brake, oorail_speed.stopped, oorail_speed.current_speed, oorail_speed.target_speed, oorail_speed.coast_speed,
    oorail_speed.max_speed);
  server.send(200, "text/plain", response);
  digitalWrite(WIFI_LED, 0);  
}

void oorail_web_askspeed() {
  char response[255];
  digitalWrite(WIFI_LED, 1);
  sprintf(response,"Current speed is %d. Target speed is %d", oorail_speed.current_speed, oorail_speed.target_speed);
  server.send(200, "text/plain", response);
  digitalWrite(WIFI_LED, 0);
}

void oorail_web_coast() {
  digitalWrite(WIFI_LED, 1);
  server.send(200, "text/plain", "oorail-ddc-basic coast command received");
  oorail_speed.target_speed =  oorail_speed.coast_speed;
  digitalWrite(WIFI_LED, 0);
}

void oorail_web_shunt() {
  digitalWrite(WIFI_LED, 1);
  server.send(200, "text/plain", "oorail-ddc-basic shunt command received");
  oorail_speed.target_speed = OORAIL_DEFAULT_SHUNT;
  digitalWrite(WIFI_LED, 0);  
}

void oorail_web_stop() {
  digitalWrite(WIFI_LED, 1);
  server.send(200, "text/plain", "oorail-ddc-basic stop command received");
  oorail_speed.target_speed = 0;
  digitalWrite(WIFI_LED, 0);
}

void oorail_web_estop() {
  digitalWrite(WIFI_LED, 1);
  server.send(200, "text/plain", "oorail-ddc-basic estop command received");
  oorail_speed.target_speed = 0;
  oorail_ctl_set_speed(0);
  digitalWrite(WIFI_LED, 0);
}

void oorail_web_brakeon() {
  digitalWrite(WIFI_LED, 1);
  if (oorail_speed.brake == 0) {
    server.send(200, "text/plain", "oorail-ddc-basic apply brake command received");
    oorail_speed.brake = 1;
  } else {
    server.send(403, "text/plain", "Forbidden Brake Already On");    
  }
  digitalWrite(WIFI_LED, 0);
}

void oorail_web_brakeoff() {
  digitalWrite(WIFI_LED, 1);
  if (oorail_speed.brake == 1) {
    server.send(200, "text/plain", "oorail-ddc-basic release brake command received");
    oorail_speed.brake = 0;
  } else {
    server.send(403, "text/plain", "Forbidden Brake Already Off");    
  }
  digitalWrite(WIFI_LED, 0);
}

void oorail_web_brake() {
  digitalWrite(WIFI_LED, 1);
  if (oorail_speed.brake == 0) {
    server.send(200, "text/plain", "oorail-ddc-basic apply brake command received");
    if ((oorail_speed.current_speed - OORAIL_DEFAULT_BRAKE) >= 0) {
      oorail_ctl_set_speed(oorail_speed.current_speed - OORAIL_DEFAULT_BRAKE);
    } else {
      oorail_ctl_set_speed(0);
    }
  } else {
    server.send(403, "text/plain", "Forbidden Brake Already On");    
  }
  digitalWrite(WIFI_LED, 0);
}


void oorail_web_accel() {
  digitalWrite(WIFI_LED, 1); 
  if (oorail_speed.current_speed + OORAIL_DEFAULT_SPEEDSTEP <= oorail_speed.max_speed) {
    oorail_speed.target_speed = oorail_speed.current_speed + OORAIL_DEFAULT_SPEEDSTEP; 
    server.send(200, "text/plain", "oorail-ddc-basic accelerate command received");
  } else if (oorail_speed.current_speed + 1 <= oorail_speed.max_speed) {
    oorail_speed.target_speed = oorail_speed.current_speed + 1; 
    server.send(200, "text/plain", "oorail-ddc-basic accelerate command received");   
  } else {
    server.send(403, "text/plain", "Forbidden - Maximum Speed Reached");       
  }
  digitalWrite(WIFI_LED, 0);
}

void oorail_web_decel() {
  Serial.println("**DEBUG**: received deceleration request");
  digitalWrite(WIFI_LED, 1);
  server.send(200, "text/plain", "oorail-ddc-basic decelerate command received");
  if (oorail_speed.current_speed != 0) {
    if (oorail_speed.current_speed  >= OORAIL_DEFAULT_SPEEDSTEP) {
      Serial.println("**DEBUG**: SPEEDSTEP");
      oorail_speed.target_speed = oorail_speed.current_speed - OORAIL_DEFAULT_SPEEDSTEP;
    } else {
      Serial.println("**DEBUG**: SMALLSTEP");
      oorail_speed.target_speed = oorail_speed.current_speed - 1;
    }
  } else {
    Serial.println("**DEBUG**: current speed is zero");
  }
  digitalWrite(WIFI_LED, 0);
}

void oorail_web_handler_root() {
  digitalWrite(WIFI_LED, 1);
  server.send(200, "text/plain", "oorail-ddc-basic is OK");
  digitalWrite(WIFI_LED, 0);
}

void oorail_web_err_notfound() {
  digitalWrite(WIFI_LED, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(WIFI_LED, 0);
}

/* END HTTP SERVER */

void oorail_wifi()
{
  Serial.println(" * Connecting to WiFi... ");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }
}

/* Motor  Control */
void oorail_ctl_set_speed(unsigned int speed) {
  Serial.print("Speed ");
  Serial.print(speed);
  Serial.println("");

  ledcWrite(mchan, speed);
  oorail_speed.current_speed = speed;
}

void oorail_ctl_speed() {
  if (oorail_speed.target_speed == oorail_speed.current_speed) return;

  Serial.println("**DEBUG**: speed ctl");
  if (oorail_speed.target_speed > oorail_speed.current_speed) {
    /* Accelerating */
    /* Check if we are incrementing single step or speed step */
    Serial.println("**DEBUG**: Accelerating");
    if (oorail_speed.current_speed + OORAIL_DEFAULT_SPEEDSTEP <= oorail_speed.target_speed) {
      oorail_ctl_set_speed(oorail_speed.current_speed + OORAIL_DEFAULT_SPEEDSTEP);
    } else {
      oorail_ctl_set_speed(oorail_speed.current_speed + 1);
    }
  } else if (oorail_speed.target_speed < oorail_speed.current_speed) {
    /* Decelerating */
    /* Check if we are using single or speed step */
    Serial.println("**DEBUG**: deceleration.....");
    if (oorail_speed.current_speed - OORAIL_DEFAULT_SPEEDSTEP  >= oorail_speed.target_speed) {
     Serial.println("**DEBUG**: decel-1");
      oorail_ctl_set_speed(oorail_speed.current_speed - OORAIL_DEFAULT_SPEEDSTEP);
    } else {
      Serial.println("**DEBUG**: decel-2");
      oorail_ctl_set_speed(oorail_speed.current_speed - 1);
    }
  } else {
    /* Problem */
    Serial.println("WATCHDOG in oorail_ctl_speed");
    Serial.print("Target and Current Speed mismatch ");
    Serial.print(oorail_speed.target_speed);
    Serial.print(" ");
    Serial.print(oorail_speed.current_speed);
    Serial.println(" ");
    return;
  }
  Serial.println("");
  Serial.print("Target Speed = ");
  Serial.print(oorail_speed.target_speed);
  Serial.print(" | Current Speed = ");
  Serial.println(oorail_speed.current_speed);
  delay(450);
  if ((oorail_speed.current_speed == 0) && (oorail_speed.stopped = 0)) {
    oorail_speed.stopped = 1;
  } else if ((oorail_speed.current_speed > 0)  && (oorail_speed.stopped == 1)) {
    oorail_speed.stopped = 0;
  }
}

void oorail_speed_init() {
  int i = 0;
  Serial.println(" * Initializing Profile ");

  oorail_speed.brake = 1;
  oorail_speed.stopped = 1;
  oorail_speed.current_speed = 0;
  oorail_speed.coast_speed = (OORAIL_DEFAULT_MAXSPEED / 2);
  oorail_speed.target_speed = 0;
  oorail_speed.max_speed = OORAIL_DEFAULT_MAXSPEED;

  oorail_mph = oorail_speed.current_speed;
}

void oorail_setup_l283d(void)
{
  Serial.println(" * Initializing L283d module...");
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  ledcSetup(mchan, mfreq, mres);
  ledcAttachPin(ENA, mchan);
  ledcWrite(mchan, 0); // start stopped
}

void setup() {
  pinMode(WIFI_LED, OUTPUT);
  Serial.begin (115200);
  oorail_banner();
  oorail_speed_init();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  oorail_wifi();
  oorail_web_init();
  oorail_setup_l283d();
}

void loop() {
  /* Configure L283N to go forwards */
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  oorail_ctl_speed();
  server.handleClient();
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
