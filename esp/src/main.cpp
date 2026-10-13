#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <stdio.h>
#include "IPAddress.h"
#include "config.h"
#include "crypto.hpp"

WiFiServer server(CONFIG_PORT);
WOLCrypto crypto;

void startWifi();
void sendWol();

void setup() {
    // initialize LED digital pin as an output.
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(9600);

    crypto.init(CONFIG_KEY, sizeof(CONFIG_KEY));

    startWifi();
    puts("wifi connected\n");
    server.begin();
    printf("Listening on %s, port %d\n", WiFi.localIP().toString().c_str(), CONFIG_PORT);
}

void loop() {
    WiFiClient client = server.accept();
    if (!client) {
        delay(1);
        return;
    }
    puts("client ready");

    // the client is connected - send the challenge
    uint8_t challenge[CHALLENGE_LEN];
    crypto.getChallenge(challenge);

    client.write(challenge, sizeof(challenge));
    client.flush();

    auto timeoutMillis = millis() + 1000;
    while (client.available() < RESPONSE_LEN) {
        delay(1);
        if (millis() > timeoutMillis) {
            puts("challenge reply timeout");
            client.stop();
            return;
        }
    }

    uint8_t response[RESPONSE_LEN];
    auto len = client.read(response, sizeof(response));
    if (len == RESPONSE_LEN && crypto.verifyChallenge(challenge, response) == true) {
        // ok
        sendWol();
        client.write((uint8_t*)"\nOK\n", 4);
    } else {
        // not ok
        client.write((uint8_t*)"\nNO\n", 4);
    }

    client.flush();
    client.stop();
    puts("client disconnected\n");
}


void startWifi() {
    WiFi.begin(CONFIG_WIFI_SSID, CONFIG_WIFI_PASSWORD);

    int8_t recheckCount = 30;
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        if (--recheckCount <= 0) {
            puts("Connection timeout, resetting\n");
            ESP.reset();
        }

        puts("Not connected, still waiting...\n");
    }
}

void sendWol() {
    WiFiUDP udp;
    byte preamble[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    udp.beginPacket(WiFi.broadcastIP(), 9);
    udp.write(preamble, sizeof(preamble));
    for (uint8 i = 0; i < 16; i++) {
        udp.write(CONFIG_TARGET_MAC, sizeof(CONFIG_TARGET_MAC));
    }
    udp.endPacket();
}
