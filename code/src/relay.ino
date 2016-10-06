/*

ESPurna
RELAY MODULE

Copyright (C) 2016 by Xose Pérez <xose dot perez at gmail dot com>

*/

#include <EEPROM.h>

// -----------------------------------------------------------------------------
// RELAY
// -----------------------------------------------------------------------------

void switchRelayOn() {

    if (!digitalRead(RELAY_PIN)) {

        DEBUG_MSG("[RELAY] ON\n");
        digitalWrite(RELAY_PIN, HIGH);
        EEPROM.write(0, 1);
        EEPROM.commit();

        mqttSend((char *) MQTT_STATUS_TOPIC, (char *) "1");
        webSocketSend((char *) "{\"relayStatus\": true}");

    }

}

void switchRelayOff() {

    if (digitalRead(RELAY_PIN)) {

        DEBUG_MSG("[RELAY] OFF\n");
        digitalWrite(RELAY_PIN, LOW);
        EEPROM.write(0, 0);
        EEPROM.commit();

        mqttSend((char *) MQTT_STATUS_TOPIC, (char *) "0");
        webSocketSend((char *) "{\"relayStatus\": false}");

    }

}

void toggleRelay() {
    if (digitalRead(RELAY_PIN)) {
        switchRelayOff();
    } else {
        switchRelayOn();
    }
}

void relaySetup() {
    pinMode(RELAY_PIN, OUTPUT);
    EEPROM.begin(4096);
    EEPROM.read(0) == 1 ? switchRelayOn() : switchRelayOff();
}
