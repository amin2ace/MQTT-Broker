#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// MQTT Configuration
#define MQTT_MAX_PACKET_SIZE 256  // Maximum size for MQTT messages
const char* MQTT_BROKER = "your_broker_address";
const int MQTT_PORT = 1883;
const char* MQTT_USERNAME = "your_username";
const char* MQTT_PASSWORD = "your_password";
const char* MQTT_CLIENT_ID = "ESP8266_Client";

// WiFi client and MQTT client instances
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// Function declarations
bool connectToMQTT();
void mqttCallback(char* topic, byte* payload, unsigned int length);
bool publishMessage(const char* topic, const char* message, bool retain = false);
bool subscribeToTopic(const char* topic, uint8_t qos = 0);

void setup() {
    // Initialize Serial for debugging
    Serial.begin(115200);
    Serial.println("\nInitializing...");

    // Connect to MQTT broker
    while (!connectToMQTT()) {
        delay(5000);  // Wait 5 seconds before retrying
    }
}

void loop() {
    // Check if MQTT connection is still alive
    if (!mqttClient.connected()) {
        connectToMQTT();
    }
    
    // Process MQTT messages
    mqttClient.loop();
}

/**
 * MQTT callback function for handling incoming messages
 * @param topic    The topic of the received message. Points to a character array containing
 *                the topic string. The topic indicates the channel/subject of the message.
 * @param payload  Pointer to the raw message data as a byte array. Contains the actual
 *                message content that was received. Not null-terminated.
 * @param length   The length of the payload in bytes. Use this to know how many bytes to
 *                read from the payload array since it's not null-terminated.
 */
void mqttCallback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();
}

// Function to connect to MQTT broker with error handling
bool connectToMQTT() {
    Serial.println("Attempting MQTT connection...");
    
    // Set the MQTT broker details
    mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
    mqttClient.setCallback(mqttCallback);
    
    // Attempt to connect with credentials
    if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD)) {
        Serial.println("Connected to MQTT broker successfully");
        
        // Subscribe to topics here if needed
        // mqttClient.subscribe("your/topic");
        
        return true;
    } else {
        Serial.print("Failed to connect to MQTT broker, rc=");
        Serial.print(mqttClient.state());
        Serial.println(" Retry in 5 seconds");
        
        // Print detailed error message based on return code
        switch (mqttClient.state()) {
            case -4:
                Serial.println("MQTT_CONNECTION_TIMEOUT");
                break;
            case -3:
                Serial.println("MQTT_CONNECTION_LOST");
                break;
            case -2:
                Serial.println("MQTT_CONNECT_FAILED");
                break;
            case -1:
                Serial.println("MQTT_DISCONNECTED");
                break;
            case 1:
                Serial.println("MQTT_CONNECT_BAD_PROTOCOL");
                break;
            case 2:
                Serial.println("MQTT_CONNECT_BAD_CLIENT_ID");
                break;
            case 3:
                Serial.println("MQTT_CONNECT_UNAVAILABLE");
                break;
            case 4:
                Serial.println("MQTT_CONNECT_BAD_CREDENTIALS");
                break;
            case 5:
                Serial.println("MQTT_CONNECT_UNAUTHORIZED");
                break;
        }
        return false;
    }
}

/**
 * Publishes a message to a specified MQTT topic
 * @param topic The topic to publish to
 * @param message The message to publish
 * @param retain Whether to retain the message on the broker (default: false)
 * @return boolean indicating success/failure
 */
bool publishMessage(const char* topic, const char* message, bool retain) {
    if (!mqttClient.connected()) {
        Serial.println("Error: Cannot publish - MQTT client not connected");
        return false;
    }

    Serial.print("Publishing message to topic: ");
    Serial.println(topic);
    Serial.print("Message: ");
    Serial.println(message);

    // Attempt to publish
    if (mqttClient.publish(topic, message, retain)) {
        Serial.println("Message published successfully");
        return true;
    } else {
        Serial.println("Error: Failed to publish message");
        Serial.print("Message length: ");
        Serial.println(strlen(message));
        
        if (strlen(message) > MQTT_MAX_PACKET_SIZE) {
            Serial.println("Error: Message too large for MQTT packet size");
        }
        return false;
    }
}

/**
 * Subscribes to a specified MQTT topic
 * @param topic The topic to subscribe to
 * @param qos Quality of Service level (0, 1, or 2)
 * @return boolean indicating success/failure
 */
bool subscribeToTopic(const char* topic, uint8_t qos) {
    if (!mqttClient.connected()) {
        Serial.println("Error: Cannot subscribe - MQTT client not connected");
        return false;
    }

    Serial.print("Subscribing to topic: ");
    Serial.println(topic);
    Serial.print("QoS level: ");
    Serial.println(qos);

    // Attempt to subscribe
    if (mqttClient.subscribe(topic, qos)) {
        Serial.println("Successfully subscribed to topic");
        return true;
    } else {
        Serial.println("Error: Failed to subscribe to topic");
        return false;
    }
}