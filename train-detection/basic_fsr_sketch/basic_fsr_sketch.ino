
    int fsrPin = 0;     // the FSR and 10K pulldown are connected to a0
    int fsrReading;     // the analog reading from the FSR resistor divider
     
    void setup(void) {
      // We'll send debugging information via the Serial monitor
      Serial.begin(9600);   
    }
     
    void loop(void) {
      fsrReading = analogRead(fsrPin);  
     
      Serial.print("Analog reading = ");
      Serial.print(fsrReading);     // the raw analog reading
     
      // We'll have a few threshholds, qualitatively determined
      if (fsrReading < 10) {
        Serial.println(" - Track vacant");
      } else if (fsrReading < 80) {
        Serial.println(" - Freight Wagon");
      } else if (fsrReading < 200) {
        Serial.println(" - Class 03 Shunter");
      } else if (fsrReading < 400) {
        Serial.println(" - Class 27 Diesel");
      } else {
        Serial.println(" - Something heavier");
      }
      delay(1000);
    } 
