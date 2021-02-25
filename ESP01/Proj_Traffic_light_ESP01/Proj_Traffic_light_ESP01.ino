#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>


const char auth[] = "Blynk-Authentication-Code";
const char ssid[] = "WiFi-Name";
const char pass[] = "WiFi-Password";

// To store the traffic light button values coming from the Blynk app.
int red, amber, green;

int sequence;  // For the sequence button on the Blynk app.

String spacer = " ";  // Used to seperate the button values.



void setup() {
    Serial.begin(19200);
    Blynk.begin(auth, ssid, pass);
}



// Getting the button values from the Blynk app.
BLYNK_WRITE(V0) {
    red = param.asInt();
}
BLYNK_WRITE(V1) {
    amber = param.asInt();
}
BLYNK_WRITE(V2) {
    green = param.asInt();
}
BLYNK_WRITE(V3) {
    sequence = param.asInt();
}



void loop() {
    Blynk.run();

    // Combining all button values into one big string to send to the Attiny at once.
    String buttonValues = red + spacer + amber + spacer + green + spacer + sequence;
    Serial.println(buttonValues);

    delay(10);  // Added to make sure the Attiny can keep up and keep everything stable.
}
