// UDP push-mode test for the interrupt bridge, run on the UNO R4 WiFi's
// RA4M1.
//
// Joins WiFi with WiFiS3, then talks raw AT to the bridge on Serial2:
// AT+FWVERSION?, AT+UDPBEGIN=<port>, AT+UDPPUSH=<sock>,1. After that,
// loop() never sends a command; it only prints what the bridge sends. So
// every "+UDPRX:" line in the Serial Monitor was pushed by the bridge.
//
// Copy arduino_secrets.h.example to arduino_secrets.h and fill it in.

#include <WiFiS3.h>
#include "arduino_secrets.h"

const unsigned int UDP_PORT = 4210;

// Print bytes from the AT link readably: \r and \n shown as text (a new
// line after each \n), other control bytes as \xHH.
void printEscaped(const String &s) {
  for (unsigned int i = 0; i < s.length(); i++) {
    printEscapedByte((uint8_t)s[i]);
  }
}

void printEscapedByte(uint8_t c) {
  if (c == '\r') {
    Serial.print("\\r");
  } else if (c == '\n') {
    Serial.println("\\n");
  } else if (c < 0x20 || c > 0x7e) {
    char hex[5];
    snprintf(hex, sizeof(hex), "\\x%02X", c);
    Serial.print(hex);
  } else {
    Serial.write(c);
  }
}

// Send one AT command, then collect the reply until it ends in OK or
// ERROR, or the timeout passes. Prints both directions.
String atCommand(const String &cmd, unsigned long timeout_ms = 2000) {
  Serial.print("> ");
  Serial.println(cmd);
  Serial2.print(cmd);
  Serial2.print("\r\n");

  String reply;
  unsigned long start = millis();
  while (millis() - start < timeout_ms) {
    while (Serial2.available()) {
      reply += (char)Serial2.read();
    }
    if (reply.endsWith("OK\r\n") || reply.endsWith("ERROR\r\n")) {
      break;
    }
  }
  Serial.print("< ");
  printEscaped(reply);
  if (!reply.endsWith("\n")) {
    Serial.println("   (no complete reply)");
  }
  return reply;
}

void halt(const char *why) {
  Serial.print("STOPPED: ");
  Serial.println(why);
  while (true) {
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 5000) {
  }
  Serial.println();
  Serial.println("=== UDP push-mode test ===");

  // WiFiS3 starts the AT link with AT+SOFTRESETWIFI, which also clears any
  // push mode left on by an earlier run of this sketch.
  Serial.print("Joining WiFi ");
  Serial.println(SECRET_SSID);
  while (WiFi.begin(SECRET_SSID, SECRET_PASS) != WL_CONNECTED) {
    Serial.println("  not joined yet, retrying in 5 s");
    delay(5000);
  }
  while (WiFi.localIP() == IPAddress(0, 0, 0, 0)) {
    delay(200);
  }
  IPAddress ip = WiFi.localIP();
  Serial.print("Joined. Board IP: ");
  Serial.println(ip);

  // From here on, raw AT only; WiFiS3 is not called again.
  atCommand("AT+FWVERSION?");

  String reply = atCommand("AT+UDPBEGIN=" + String(UDP_PORT));
  int at = reply.indexOf("+UDPBEGIN: ");
  if (at < 0 || !reply.endsWith("OK\r\n")) {
    halt("AT+UDPBEGIN failed");
  }
  int sock = reply.substring(at + 11).toInt();

  reply = atCommand("AT+UDPPUSH=" + String(sock) + ",1");
  if (!reply.endsWith("OK\r\n")) {
    halt("AT+UDPPUSH was refused: this bridge has no push mode (stock firmware?)");
  }

  Serial.println();
  Serial.print("Push mode is ON for socket ");
  Serial.print(sock);
  Serial.println(". Send a datagram from the PC:");
  Serial.print("  echo -n hello | nc -u -w1 ");
  Serial.print(ip);
  Serial.print(" ");
  Serial.println(UDP_PORT);
  Serial.println("Everything below arrived unasked:");
}

void loop() {
  while (Serial2.available()) {
    printEscapedByte((uint8_t)Serial2.read());
  }
}
