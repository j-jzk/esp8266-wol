# ESP8266 Wake-on-LAN server
## Motivation
Having a home server running at all times is noisy, inefficient, and constantly spinning the HDDs might create unnecessary wear if they aren't made for it.

Many network cards provide a Wake-on-LAN feature that powers on the computer when receiving a special packet; but these packet's can't be sent from the WAN (firstly because it would be a security issue).

This project is a program that can be flashed onto an ESP8266 that listens for connections.
When a client connects and correctly answers a cryptographic challenge, the server sends a WOL packet to wake up the computer.

## How to use
The server resides in the `esp` folder.
- Create a `src/config.h` from `src/config.example.h`.
    - This includes generating a cryptographic key - save it somewhere.
- Flash it onto an ESP8266 board using `platformio run -t upload`.
- (Configure port forwarding on the router to allow access to the ESP (port 4921) from the Internet.)

* To wake up the computer, run `client.py <host> <port> <key-file>`

## How it works
When a client connects, the server sends it a cryptographic challenge (64 random bytes from the ESP8266 secure RNG).
The client then computes a HMAC-SHA512 of the challenge with a key that was generated beforehand, and sends it back (512/8 = also 64 B).
The server then verifies that the HMAC is correct, and if it is, it sends the WOL packet, and responds to the client with "\nOK\n".
If the HMAC is incorrect, it does nothing and responds with "\nNO\n".

## Limitations
- Only one WOL target (enough for me)
- The settings are compiled into the program, including the crypto key and WiFi password (but this is fine because an attacker wouldn't get physical access to the device)
- Vulnerable to a slowloris DoS attack (but who would want to DoS such a service)
