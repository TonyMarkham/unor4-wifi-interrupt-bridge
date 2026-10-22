// UDP push mode, event-driven: the RA4M1 sleeps until a datagram arrives.
//
// Needs the interrupt bridge (0.6.101 or later) on the board's ESP32-S3.
//
// setup() joins WiFi with WiFiS3, then talks raw AT to the bridge on
// Serial2: AT+FWVERSION? (prints the bridge version), AT+UDPBEGIN=<port>
// and AT+UDPPUSH=<sock>,1. From then on the bridge sends every datagram
// for that socket unasked, as
//
//   +UDPRX: <sock>,<len>|<len bytes>\r\n
//
// loop() sleeps (__WFI) whenever there is nothing to read. Only an
// interrupt wakes it: the UART receiving a byte from the bridge, or the
// UART finishing sending this sketch's own output. Each frame is parsed
// and its payload printed, together with the number of times the RA4M1
// woke up with nothing to read since the previous datagram. A count of 0
// shows it slept the whole time in between.
//
// To make the sleep real, setup() stops the core's 1 ms millis() timer
// (AGT0), which would otherwise wake the RA4M1 1000 times a second. After
// setup(), millis() and delay() stand still: do not use them in loop().
//
// Copy arduino_secrets.h.example to arduino_secrets.h and fill it in.

#include <WiFiS3.h>
#include "arduino_secrets.h"

const unsigned int UDP_PORT = 4210;

// Largest datagram the bridge forwards (WiFiUDP::parsePacket's buffer)
const size_t MAX_PAYLOAD = 1460;

// Print a byte readably: printable ASCII as is, anything else as \xHH.
void printEscapedByte(uint8_t c) {
  if (c < 0x20 || c > 0x7e) {
    char hex[5];
    snprintf(hex, sizeof(hex), "\\x%02X", c);
    Serial.print(hex);
  } else {
    Serial.write(c);
  }
}

// Send one AT command and return its reply (until OK or ERROR, or the
// timeout). Uses millis(), so it only works before the timer is stopped.
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
  for (unsigned int i = 0; i < reply.length(); i++) {
    char c = reply[i];
    if (c == '\n') {
      Serial.println();
    } else if (c != '\r') {
      printEscapedByte((uint8_t)c);
    }
  }
  return reply;
}

void halt(const char *why) {
  Serial.print("STOPPED: ");
  Serial.println(why);
  while (true) {
  }
}

// ---- +UDPRX: frame parser, fed one byte at a time from loop() ----

enum ParseState { HEADER, PAYLOAD, TRAILER };

ParseState state = HEADER;
char header[32];          // "+UDPRX: <sock>,<len>" while it arrives
size_t header_len = 0;
int frame_sock = 0;
size_t frame_len = 0;     // payload length announced by the header
size_t payload_got = 0;   // payload bytes received so far
uint8_t payload[MAX_PAYLOAD];
size_t trailer_got = 0;

// Times loop() woke up and found nothing to read, since the last report
uint32_t idle_wakeups = 0;

void reportDatagram() {
  Serial.print("datagram on socket ");
  Serial.print(frame_sock);
  Serial.print(", ");
  Serial.print(frame_len);
  Serial.print(" bytes: ");
  size_t shown = frame_len < MAX_PAYLOAD ? frame_len : MAX_PAYLOAD;
  for (size_t i = 0; i < shown; i++) {
    printEscapedByte(payload[i]);
  }
  if (shown < frame_len) {
    Serial.print(" (truncated)");
  }
  Serial.print("   [idle wake-ups since the previous datagram: ");
  Serial.print(idle_wakeups);
  Serial.println("]");

  // Wait until this output has gone out, so the UART's transmit
  // interrupts are not counted as idle wake-ups.
  Serial.flush();
  idle_wakeups = 0;
}

void parseByte(uint8_t c) {
  switch (state) {
    case HEADER:
      if (c == '|') {
        header[header_len] = '\0';
        int sock;
        unsigned int len;
        if (sscanf(header, "+UDPRX: %d,%u", &sock, &len) == 2) {
          frame_sock = sock;
          frame_len = len;
          payload_got = 0;
          state = frame_len > 0 ? PAYLOAD : TRAILER;
          trailer_got = 0;
        }
        header_len = 0;
      } else if (c == '\n') {
        // a line that is not a frame header: not expected after setup
        header_len = 0;
      } else if (header_len < sizeof(header) - 1) {
        header[header_len++] = (char)c;
      } else {
        header_len = 0;   // too long to be a header: start over
      }
      break;

    case PAYLOAD:
      if (payload_got < MAX_PAYLOAD) {
        payload[payload_got] = c;
      }
      payload_got++;
      if (payload_got == frame_len) {
        state = TRAILER;
        trailer_got = 0;
      }
      break;

    case TRAILER:
      // the "\r\n" that ends the frame
      trailer_got++;
      if (trailer_got == 2) {
        reportDatagram();
        state = HEADER;
      }
      break;
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 5000) {
  }
  Serial.println();
  Serial.println("=== UDP push mode, sleeping RA4M1 ===");

  // WiFiS3 starts the AT link with AT+SOFTRESETWIFI, which also clears any
  // push mode left on by an earlier sketch.
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
  Serial.println("Stopping the 1 ms timer; sleeping until a datagram arrives.");
  Serial.flush();

  // Stop AGT0, the core's millis() timer, so its 1 ms interrupt no
  // longer wakes the RA4M1. millis() and delay() stand still from here.
  R_AGT0->AGTCR_b.TSTART = 0;
  while (R_AGT0->AGTCR_b.TCSTF) {
  }

  idle_wakeups = 0;
}

void loop() {
  // Check for data and go to sleep with interrupts masked, so a byte that
  // arrives between the check and __WFI() cannot be missed: a pending
  // interrupt still ends __WFI(), and its handler runs at __enable_irq().
  __disable_irq();
  if (Serial2.available() == 0) {
    __WFI();
    __enable_irq();
    if (Serial2.available() == 0) {
      idle_wakeups++;
    }
    return;
  }
  __enable_irq();

  while (Serial2.available()) {
    parseByte((uint8_t)Serial2.read());
  }
}
