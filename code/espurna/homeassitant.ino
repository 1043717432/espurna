/*

HOME ASSISTANT MODULE

Copyright (C) 2017-2018 by Xose Pérez <xose dot perez at gmail dot com>

*/

#if HOMEASSISTANT_SUPPORT

#include <ArduinoJson.h>

bool _haEnabled = false;
bool _haSendFlag = false;

// -----------------------------------------------------------------------------

void _haWebSocketOnSend(JsonObject& root) {
    root["haVisible"] = 1;
    root["haPrefix"] = getSetting("haPrefix", HOMEASSISTANT_PREFIX);
}

void _haSend() {

    // Pending message to send?
    if (!_haSendFlag) return;

    // Are we connected?
    if (!mqttConnected()) return;

    DEBUG_MSG_P(PSTR("[HA] Sending autodiscovery MQTT message\n"));

    String output;

    if (_haEnabled) {

        DynamicJsonBuffer jsonBuffer;
        JsonObject& root = jsonBuffer.createObject();

        root["name"] = getSetting("hostname");
        root["platform"] = "mqtt";

        if (relayCount()) {
            root["state_topic"] = mqttGetTopic(MQTT_TOPIC_RELAY, 0, false);
            root["command_topic"] = mqttGetTopic(MQTT_TOPIC_RELAY, 0, true);
            root["payload_on"] = String("1");
            root["payload_off"] = String("0");
            root["availability_topic"] = mqttGetTopic(MQTT_TOPIC_STATUS, false);
            root["payload_available"] = String("1");
            root["payload_not_available"] = String("0");
        }

        #if LIGHT_PROVIDER != LIGHT_PROVIDER_NONE

            if (lightHasColor()) {
                root["brightness_state_topic"] = mqttGetTopic(MQTT_TOPIC_BRIGHTNESS, false);
                root["brightness_command_topic"] = mqttGetTopic(MQTT_TOPIC_BRIGHTNESS, true);
                root["rgb_state_topic"] = mqttGetTopic(MQTT_TOPIC_COLOR_RGB, false);
                root["rgb_command_topic"] = mqttGetTopic(MQTT_TOPIC_COLOR_RGB, true);
                root["color_temp_command_topic"] = mqttGetTopic(MQTT_TOPIC_MIRED, true);
            }

            if (lightChannels() > 3) {
                root["white_value_state_topic"] = mqttGetTopic(MQTT_TOPIC_CHANNEL, 3, false);
                root["white_value_command_topic"] = mqttGetTopic(MQTT_TOPIC_CHANNEL, 3, true);
            }

        #endif // LIGHT_PROVIDER != LIGHT_PROVIDER_NONE

        root.printTo(output);
    }

    #if LIGHT_PROVIDER == LIGHT_PROVIDER_NONE
        String component = String("switch");
    #else
        String component = String("light");
    #endif

    String topic = getSetting("haPrefix", HOMEASSISTANT_PREFIX) +
        "/" + component +
        "/" + getSetting("hostname") +
        "/config";

    mqttSendRaw(topic.c_str(), output.c_str());
    mqttSend(MQTT_TOPIC_STATUS, MQTT_STATUS_ONLINE, true);
    _haSendFlag = false;

}

void _haConfigure() {
    bool enabled = getSetting("haEnabled", HOMEASSISTANT_ENABLED).toInt() == 1;
    _haSendFlag = (enabled != _haEnabled);
    _haEnabled = enabled;
    _haSend();
}

// -----------------------------------------------------------------------------

void haSetup() {

    _haConfigure();

    #if WEB_SUPPORT
        wsOnSendRegister(_haWebSocketOnSend);
        wsOnAfterParseRegister(_haConfigure);
    #endif

    // On MQTT connect check if we have something to send
    mqttRegister([](unsigned int type, const char * topic, const char * payload) {
        if (type == MQTT_CONNECT_EVENT) _haSend();
    });

}

#endif // HOMEASSISTANT_SUPPORT
