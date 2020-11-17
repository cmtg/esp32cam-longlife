/*
  Copyright (c) 2020 Christian Gut. All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/

#include "esp32cam-longlife-log.h"

WiFiClientSecure client_log;

#if ULP == ULP_ON  
  const String ulp_status = "On";
#else 
  const String ulp_status = "Off";
#endif

void log_to_thingspeak(String msg) {
  StaticJsonDocument<1024> doc;  

  doc["api_key"] = THINGSSPEAK_WRITE_API_KEY;
  doc["field1"] = CAMERA_NAME;
  doc["field2"] = ulp_status;
  doc["field3"] = msg;

  if (client_log.connect(THINGSPEAK_SERVER,THINGSPEAK_PORT)) {   
    String header;
    String body;

    header = "POST /update HTTP/1.1\n";
    header += "Host: api.thingspeak.com\n";
    header += "Connection: close\n";
    header += "X-THINGSPEAKAPIKEY: ";
    header += THINGSSPEAK_WRITE_API_KEY;
    header += "\n";
    header += "Content-Type: application/json\n";
    header += "Content-Length: ";
    header += measureJson(doc);
    header += "\n";

    #if DEBUG_LEVEL >= DEBUG_VERBOSE
      Serial.println(F("Sending request header to Thingspeak server:"));
      Serial.println(header);
      Serial.println(F(""));
    #endif
    
    client_log.println(header);

    serializeJson(doc,body);
    
    #if DEBUG_LEVEL >= DEBUG_DEBUG
      Serial.println(F("Sending JSON Payload to Thingspeak server:"));
      Serial.println(body);
      Serial.println(F(""));
    #endif

    client_log.println(body);

    wait_for_response();
    
    client_log.stop();
  } else {
    #if DEBUG_LEVEL >= DEBUG_ERROR
      Serial.println(F("Connection to Thingspeak server failed!"));
    #endif
  }
}

void wait_for_response() {
  bool finishedHeaders = false;
  bool currentLineIsBlank = true;
  String body = "";
  String headers = "";
  long now;
  bool responseReceived = false;

  now = millis();
  while (millis() - now < RESPONSE_TIMEOUT) {
    while (client_log.available()) {
      char c = client_log.read();
      responseReceived = true;

      if (!finishedHeaders) {
        if (currentLineIsBlank && c == '\n') {
          finishedHeaders = true;
        } else {
          headers = headers + c;
        }
      } else {
        body = body + c;
      }

      if (c == '\n') currentLineIsBlank = true;
      else if (c != '\r') currentLineIsBlank = false;

    }

    if (responseReceived) {
      #if DEBUG_LEVEL >= DEBUG_VERBOSE
        Serial.println(F(""));
        Serial.println(F("Received Header from Thingsverse Server:"));
        Serial.println(headers);
        Serial.println(F(""));
      #endif
      #if DEBUG_LEVEL >= DEBUG_DEBUG
        Serial.println(F(""));
        Serial.println(F("Received Body from Thingsverse Server:"));
        Serial.println(body);
        Serial.println(F(""));
      #endif
      #if DEBUG_LEVEL >= DEBUG_ERROR
        if ( headers.substring(0,15) != "HTTP/1.1 200 OK" ) {
          Serial.println(F(""));
          Serial.println(F("Error: Request to Thingsspeak server failed!"));
          Serial.println(F(""));          
        }
      #endif
      return;
    }
  }  
}
