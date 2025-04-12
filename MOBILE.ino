#include <SoftwareSerial.h>

SoftwareSerial gsm(9, 10); // RX, TX

void setup() {
    Serial.begin(9600);  // Initialize Serial Monitor
    gsm.begin(9600);     // Initialize GSM module (try 19200, 38400, or 115200 if 9600 doesn't work)

    Serial.println("Testing GSM module...");
    delay(5000); // Wait for the GSM module to power on

    // Send AT command to check if the GSM module is responding
    gsm.println("AT"); // Try "AT\r" if this doesn't work
    delay(1000);

    // Check for response from the GSM module
    if (gsm.available()) {
        String response = gsm.readString();
        Serial.println("GSM Response: " + response);
        if (response.indexOf("OK") >= 0) {
            Serial.println("GSM module is ready.");
        } else {
            Serial.println("GSM module not responding correctly.");
        }
    } else {
        Serial.println("No response from GSM module.");
    }
}

void loop() {
    // Print any data received from the GSM module
    while (gsm.available()) {
        Serial.write(gsm.read());
    }
}