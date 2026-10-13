import argparse
import hmac
import socket

CHALLENGE_LEN = 64

def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("host")
    parser.add_argument("port", type=int)
    parser.add_argument("keyfile")
    return parser.parse_args()

args = parse_args()
with open(args.keyfile, "rb", buffering=0) as f:
    key = f.readall()

sock = socket.create_connection((args.host, args.port))
challenge = sock.recv(CHALLENGE_LEN)
print(f"Cryptographic challenge received")

response = hmac.digest(key, challenge, "SHA512")
print(f"Response sent")
sock.send(response)

response_code = sock.recv(4)
if response_code == b"\nOK\n":
    print("OK")
elif response_code == b"\nNO\n":
    print("Bad challenge response, do you have the correct key?")
else:
    print(f"Unexpected server response: {response_code}")

sock.close()
