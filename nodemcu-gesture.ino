#include <Wire.h>
#include <SparkFun_APDS9960.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266mDNS.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiAP.h>
#include <ESP8266WiFiGeneric.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFiScan.h>
#include <ESP8266WiFiSTA.h>
#include <ESP8266WiFiType.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>

#define USE_SERIAL Serial

ESP8266WiFiMulti WiFiMulti;

#define APDS9960_INT    D6  //AKA GPIO12 -- Interupt pin
#define APDS9960_SDA    D3  //AKA GPIO0
#define APDS9960_SCL    D1  //AKA GPIO5

String JENKINS_HOST = "192.168.0.100";
String JENKINS_PORT = "8080";
String JENKINS_USER = "admin";
String JENKINS_PASSWORD = "jenkins";
String JENKINS_JOB = "buildpi";
String JENKINS_BUILD_TOKEN = "php-srb-2017";
String RASPBERRY_HOSTNAME = "jenkins9";

SparkFun_APDS9960 apds = SparkFun_APDS9960();
volatile bool isr_flag = 0;

String crumb;
String crumb_value;

char hostString[16] = {0};

void setup() {

  WiFi.begin("Quantox1", "14cd918ac");
  
  pinMode(LED_BUILTIN, OUTPUT);
  
  Wire.begin(APDS9960_SDA, APDS9960_SCL);

  pinMode(APDS9960_INT, INPUT);
  
  Serial.begin(115200);

  attachInterrupt(APDS9960_INT, interruptRoutine, FALLING);

  if ( apds.init() ) {
    Serial.println(F("APDS-9960 initialization complete"));
  } else {
    Serial.println(F("Something went wrong during APDS-9960 init!"));
  }

  if ( apds.enableGestureSensor(true) ) {
    Serial.println(F("Gesture sensor is now running"));
  } else {
    Serial.println(F("Something went wrong during gesture sensor init!"));
  }

}

void loop() {
  if ( isr_flag == 1 ) {
    detachInterrupt(APDS9960_INT);
    handleGesture();
    isr_flag = 0;
    attachInterrupt(APDS9960_INT, interruptRoutine, FALLING);
  }
}

void interruptRoutine() {
  isr_flag = 1;
}

void handleGesture() {
  if ( apds.isGestureAvailable() ) {
    switch ( apds.readGesture() ) {
      case DIR_LEFT:
        sendRequest();
        break;
      default:
        Serial.println("No valid trigger detected.");
    }
  }
}

void sendRequest() {
  if ((WiFi.status() == WL_CONNECTED)) {
    Serial.println("Sending request to " + JENKINS_HOST);
    HTTPClient http;
    http.begin("http://" + JENKINS_USER + ":" + JENKINS_PASSWORD + "@" + JENKINS_HOST + ":" + JENKINS_PORT + "/crumbIssuer/api/xml?xpath=concat(//crumbRequestField,\":\",//crumb)");
    int httpCode = http.GET();
    if (httpCode < 0) {
      USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    crumb = http.getString();
    crumb_value = crumb.substring(14);
    http.end();
    digitalWrite(LED_BUILTIN, HIGH);
    triggerBuild(crumb_value);
    digitalWrite(LED_BUILTIN, LOW);
  }
}

void triggerBuild(String crumb_value)
{
  Serial.println("Triggering build...");
  HTTPClient http;
  http.begin("http://" + JENKINS_USER + ":" + JENKINS_PASSWORD + "@" + JENKINS_HOST + ":" + JENKINS_PORT + "/job/" + JENKINS_JOB + "/build?token=" + JENKINS_BUILD_TOKEN);
  http.addHeader("Jenkins-Crumb", crumb_value);
  http.POST("");
  http.end();
}
