// TLS check for the bridge's certificate bundle, run on the UNO R4 WiFi's
// RA4M1.
//
// Every WiFiSSLClient connection that does not set its own CA certificate
// is verified by the ESP32-S3 bridge against the root certificates built
// into its firmware. This sketch joins WiFi, prints the bridge version,
// and opens a TLS connection to each host below. It expects the trusted
// ones to connect and the untrusted one to be refused, and ends with one
// summary line.
//
// The hosts cover the root certificates real servers chain to: Google,
// Cloudflare and arduino.cc send GTS Root R4 cross-signed by GlobalSign
// Root CA; GitHub and Sectigo chain to USERTrust; Let's Encrypt to ISRG;
// Arduino's cloud API to Starfield. untrusted-root.badssl.com uses a root
// no bundle contains, so a bridge that verifies at all must refuse it.
//
// Copy arduino_secrets.h.example to arduino_secrets.h and fill it in.

#include <WiFiS3.h>
#include "arduino_secrets.h"

struct Check {
  const char *host;
  bool expect_trusted;
};

const Check CHECKS[] = {
  {"www.google.com", true},
  {"www.cloudflare.com", true},
  {"www.arduino.cc", true},
  {"api2.arduino.cc", true},
  {"github.com", true},
  {"letsencrypt.org", true},
  {"www.sectigo.com", true},
  {"untrusted-root.badssl.com", false},
};

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 5000) {
  }
  Serial.println();
  Serial.println("=== TLS check of the bridge's certificate bundle ===");

  Serial.print("Joining WiFi ");
  Serial.println(SECRET_SSID);
  while (WiFi.begin(SECRET_SSID, SECRET_PASS) != WL_CONNECTED) {
    Serial.println("  not joined yet, retrying in 5 s");
    delay(5000);
  }
  while (WiFi.localIP() == IPAddress(0, 0, 0, 0)) {
    delay(200);
  }
  Serial.print("Joined. Bridge firmware: ");
  Serial.println(WiFi.firmwareVersion());
  Serial.println();

  int unexpected = 0;
  for (const Check &c : CHECKS) {
    WiFiSSLClient client;
    bool trusted = client.connect(c.host, 443);
    client.stop();

    bool as_expected = (trusted == c.expect_trusted);
    if (!as_expected) {
      unexpected++;
    }

    Serial.print(as_expected ? "  ok   " : "  FAIL ");
    Serial.print(c.host);
    Serial.print(": ");
    Serial.print(trusted ? "trusted" : "refused");
    Serial.print(" (expected ");
    Serial.print(c.expect_trusted ? "trusted" : "refused");
    Serial.println(")");
  }

  Serial.println();
  if (unexpected == 0) {
    Serial.println("RESULT: all as expected");
  } else {
    Serial.print("RESULT: ");
    Serial.print(unexpected);
    Serial.println(" unexpected");
  }
}

void loop() {
}
