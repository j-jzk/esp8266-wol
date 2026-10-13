#pragma once
#include <bearssl/bearssl.h>
#include <Esp.h>
#include "config.h"

class WOLCrypto {
    br_hmac_key_context keyContext;

    bool areBuffersEqual(const uint8_t *a, const uint8_t* b, size_t len) const {
        // theoretically it doesn't matter if we used memcmp (there should be no
        // timing attacks possible on the challenge response), but we implement
        // a safe function just for good measure
        bool isEqual = true;
        while (len-- > 0) {
            isEqual = (*a++ == *b++) && isEqual;
        }
        return isEqual;
    }

public:

    void init(const uint8_t *key, size_t keyLen) {
        br_hmac_key_init(&keyContext, &br_sha512_vtable, key, keyLen);
    }

    /**
     * Generates the cryptographic challenge (= random bytes)
     * @param buffer must be able to hold CHALLENGE_LEN bytes
     */
    void getChallenge(uint8_t *buffer) const {
        // for (int i = 0; i < CHALLENGE_LEN; i++) {
        //     buffer[i] = (uint8_t) *((uint32_t*)0x3FF20E44L);  // ESP8266 RNG register
        // }
        ESP.random(buffer, CHALLENGE_LEN);
    }

    bool verifyChallenge(uint8_t *challenge, const uint8_t *response) const {
        br_hmac_context ctx;
        br_hmac_init(&ctx, &keyContext, 0);

        br_hmac_update(&ctx, challenge, CHALLENGE_LEN);
        uint8_t expectedResponse[RESPONSE_LEN] = {0};
        br_hmac_out(&ctx, expectedResponse);

        return areBuffersEqual(response, expectedResponse, sizeof(expectedResponse));
    }
};
