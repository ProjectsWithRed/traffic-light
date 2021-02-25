#include <SoftwareSerial.h>

#define RX 3
#define TX 5
SoftwareSerial Serial1(RX, TX);


const int BTN_PIN = 4;
const int TIME_TO_ACTIVATE = 1000;  // How long button needs to be pressed until considered held down in ms.

const int LED_PINS[] = {2, 1, 0};
const int N_LEDs = sizeof(LED_PINS) / sizeof(LED_PINS[0]);  // Number of LEDs in the traffic light.


// Modes of the traffic light. A mode contains the states(HIGH/LOW) for all LEDs.
const int MODES[][N_LEDs] = {
    {1, 0, 0},  // red
    {1, 1, 0},  // red, amber
    {0, 0 ,1},  // green
    {0, 1, 0}   // amber
};
const int N_MODES = sizeof(MODES) / sizeof(MODES[0]);




// An automatic sequence. Sequences are performed automatically using a fixed delay between each mode.
// Sequence 1 will perform a regular traffic light sequence starting from a red light.
const int SEQUENCE1[][N_LEDs] = {
    {1, 0, 0},
    {1, 1, 0},
    {0, 0, 1}
};
const int SEQ_DELAY = 2000;  // Delay between traffic light mode when doing a sequence in ms.


int mode_i = 0;  // Current mode of the traffic light, chosen from MODES.

// To keep track of how many times the button is pressed.
int buttonState = 0;
int lastState = 1;


// Used to detect when button is held down.
int btn_time_pressed = 0;
// To prevent multiple button hold down activations.
bool button_activated = false;  // True when the button has been held down for LIGHT_DELAY.



// Red, amber, and green button values coming from the ESP-01(Blynk, app on phone).
int redBApp;
int amberBApp;
int greenBApp;
int seqApp;

String lastAppValues = "";
String spacer = " ";



void setup() {
    Serial1.begin(19200);

    pinMode(2, OUTPUT);
    pinMode(1, OUTPUT);
    pinMode(0, OUTPUT);

    pinMode(BTN_PIN, INPUT_PULLUP);
}





// Will return 0/1 if the button was held down for a certain amount of time "timeToActivate" in ms.
bool buttonHeldDown(int timeToActivate) {
    if (buttonState != lastState) {
        btn_time_pressed = millis();
    }
    
    if(buttonState == LOW && !button_activated) {
        int btn_pressed_time = millis() - btn_time_pressed;
        if(btn_pressed_time >= timeToActivate) {
            button_activated = true;
            return 1;
        }
    }
    
    if(buttonState == HIGH) {
        button_activated = false;
    }

    return 0;
}



// Updates the mode of the traffic light based on the button press.
void updateMode() {
    
    if(buttonState == LOW && lastState != LOW) {
        mode_i += 1;
        lastState = LOW;
        
        updateTrafficLight(MODES[mode_i]);
    }

    lastState = buttonState;

    // Reset mode
    if(mode_i == N_MODES) {
        mode_i = 0;
    }
}

// Sets the mode of the traffic light based on the given array.
void updateTrafficLight(int mode[]) {
    for(int i = 0; i < N_LEDs; i++) {
        int state = mode[i];
        digitalWrite(LED_PINS[i], state);
    }       
}


// Performs the given light sequence.
void performLightSequence(int sequence[][N_LEDs], int n_modes, int seq_delay) {
    for(int i = 0; i < n_modes; i++) {
        updateTrafficLight(sequence[i]);
        
        delay(seq_delay);
    }
}


// Processes the button states from the Blynk app coming from the ESP-01.
void processAppButtons() {
    if(Serial1.available()) {
        String raw = Serial1.readStringUntil('\n');

        // Extracting red, amber, and green button values coming from the ESP(app).         
        raw.trim();
        char* c = const_cast<char*>(raw.c_str());

        sscanf(c, "%d %d %d %d", &redBApp, &amberBApp, &greenBApp, &seqApp);

        // Updating traffic light only if values received from the ESP are not the same.
        String appValues = redBApp + spacer + amberBApp + spacer + greenBApp + spacer + seqApp;
        
        if(appValues != lastAppValues) {
            if(seqApp) {
                int n_modes = sizeof(SEQUENCE1) / sizeof(SEQUENCE1[0]);
                performLightSequence(SEQUENCE1, n_modes, SEQ_DELAY);
            } else {
                int mode[] = {redBApp, amberBApp, greenBApp};
                updateTrafficLight(mode);
            }
            lastAppValues = appValues;
        }
    }
}




void loop() {
    buttonState = digitalRead(BTN_PIN);

    int wasButtonHeldDown = buttonHeldDown(TIME_TO_ACTIVATE);

    updateMode();
    
    if(wasButtonHeldDown) {
        int n_modes = sizeof(SEQUENCE1) / sizeof(SEQUENCE1[0]);
        performLightSequence(SEQUENCE1, n_modes, SEQ_DELAY);
    }
    
    processAppButtons();
}
