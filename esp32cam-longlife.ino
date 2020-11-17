/*
  Copyright (c) 2020 Christian Gut. All right reserved.

  This camera bot is inspired, and based, on the camera bot 

   written by Giacarlo Bacchio (Gianbacchio on Github)
   adapted by Brian Lough
   bodged by Robot Zero One

  It provides additional features like:

    - a refactoring of the code as an attempt to make it more 
      readable
    - the possibility to activate the camera via an PIR sensor
    - the possibility to send the camera into deepsleep in order
      to optimize its battery life (hence the project name)

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

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "settings.h"

#include "esp32cam-longlife-cam.h"
#include "esp32cam-longlife-bot.h"
#include "esp32cam-longlife-log.h"

void setup()
{

  Serial.begin(115200);

  setup_pir_sensor();
  configure_camera();
  connect_wifi();

  #if ULP == ULP_OFF
    log_to_thingspeak("Device booted");
  #endif   


  #if ULP == ULP_ON        
    // Verifique porque o esp32 acordou 
    #if DEBUG_LEVEL >= DEBUG_DEBUG
       Serial.println(F(""));
       Serial.println(F("Discovering wakeup cause:"));
    #endif
     
    esp_sleep_wakeup_cause_t wakeup_reason;
    wakeup_reason = esp_sleep_get_wakeup_cause();

    #if DEBUG_LEVEL >= DEBUG_DEBUG
       Serial.println(get_wakeup_cause_text(wakeup_reason));
    #endif
   
   // Inicia o envio da alerta ou verifique se há comandos no Telegram
   switch(wakeup_reason)
      {
        case ESP_SLEEP_WAKEUP_EXT0: 
            send_alerts(); 
            break;
        case ESP_SLEEP_WAKEUP_TIMER: 
            check_telegram_commands(); 
            break;
        default:
            log_to_thingspeak("Device booted");
            break;
      }

    // Configure o sensor PIR como fonte de acordar o ESP32
    #if DEBUG_LEVEL >= DEBUG_DEBUG
      Serial.print(F("Setting wake-up sensor on GPIO No. "));
      Serial.println(PIR_SENSOR_GPIO);
    #endif
    esp_sleep_enable_ext0_wakeup(PIR_SENSOR_GPIO,1);

    // Configure o timer para ler se há um comando
    #if DEBUG_LEVEL >= DEBUG_DEBUG
      Serial.print(F("Setting intervall to check for commands to: "));
      Serial.print(CHECK_COMMAND_INTERVALL);
      Serial.println(F("s"));
    #endif
    
    esp_sleep_enable_timer_wakeup(CHECK_COMMAND_INTERVALL * uS_TO_S_FACTOR);
    
    // Desliga a memoria RTC reduzir o consumo de energia durante o deepsleep
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_AUTO);

    
    //Go to sleep now
    #if DEBUG_LEVEL >= DEBUG_INFO
      Serial.println(F("Entering Deep Sleep..."));
      Serial.println(F("------------------------------------------------------"));
      Serial.println(F(""));
      Serial.println(F(""));
      Serial.println(F("")); 
    #endif
    
    #if DEBUG_LEVEL >= DEBUG_OFF
      delay(200);
    #endif
    
    esp_deep_sleep_start();
  
  #endif 
}


void loop() {
  # if ULP == ULP_OFF
      // A última vez quando houve um rodada
      long lasttime_pir = 0;    
      long lasttime_command = 0;
      int pir_status;    
      camera_fb_t * fb;
    
      // Define como delay o menor tempo entre o PIR_READ_INTERVALL e CHECK_COMMAND_INTERVALL
      # if PIR_READ_INTERVALL < CHECK_COMMAND_INTERVALL
        delay(PIR_READ_INTERVALL*mS_TO_S_FACTOR);
      # else
        delay(CHECK_COMMAND_INTERVALL*mS_TO_S_FACTOR);
      # endif 
      
      if (millis() > lasttime_pir + PIR_READ_INTERVALL*mS_TO_S_FACTOR)  {
        
        pir_status = digitalRead(PIR_SENSOR_GPIO);
    
        #if DEBUG_LEVEL >= DEBUG_DEBUG
          Serial.println(F(""));
          Serial.print(F("PIR Sensor Status: "));
          Serial.println(pir_status);
          Serial.println(F(""));
        #endif
    
        if (pir_status) {
          send_alerts();
          }
  
        lasttime_pir = millis();
        } 
    
      if (millis() > lasttime_command + CHECK_COMMAND_INTERVALL*mS_TO_S_FACTOR)  {
        check_telegram_commands();
        lasttime_command = millis();
        }
  #endif
}


//Function that prints the reason by which ESP32 has been awaken from sleep
String get_wakeup_cause_text(esp_sleep_wakeup_cause_t wakeup_reason){
  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0  : return "Wakeup caused by external signal using RTC_IO" ; break;
    case ESP_SLEEP_WAKEUP_EXT1  : return "Wakeup caused by external signal using RTC_CNTL"; break;
    case ESP_SLEEP_WAKEUP_TIMER  : return "Wakeup caused by timer"; break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD  : return "Wakeup caused by touchpad"; break;
    case ESP_SLEEP_WAKEUP_ULP  : return "Wakeup caused by ULP program"; break;
    default : return "Wakeup was not caused by deep sleep"; break;
  }
}

// Envia uma alerta com mensagem de texto
// Depois envia uma imagem cada ALERT_REMINDER_INTERVALL até o sensor não detecta mais um movimento
void send_alerts() {
    #if DEBUG_LEVEL >= DEBUG_INFO
      Serial.println(F(""));
      Serial.print(F("Sending alert to chat_id "));
      Serial.println(ALERT_CHAT_ID);
      Serial.println(F(""));
   #endif
   
  log_to_thingspeak("PIR sensor detected movement, sending Telegram alert and a picture");
  
  String msg = "Alerta: Movimento detectado pela camera ";
  msg.concat(CAMERA_NAME);
  send_text(ALERT_CHAT_ID,msg);
  
  camera_fb_t * fb;
  int pir_status;   
  int lasttime_alert = -1*ALERT_REMINDER_INTERVALL*mS_TO_S_FACTOR;
  int lasttime_pir = millis();
  
  do {
    if (millis() >= lasttime_alert + ALERT_REMINDER_INTERVALL*mS_TO_S_FACTOR) {
  
      #if DEBUG_LEVEL >= DEBUG_INFO
        Serial.println(F(""));
        Serial.print(F("Sending image to chat_id "));
        Serial.println(ALERT_CHAT_ID);
        Serial.println(F(""));
      #endif
      
      fb = NULL;
      fb = take_picture();
      if ( fb ) {
        send_picture(ALERT_CHAT_ID,fb); 
        esp_camera_fb_return(fb);
      } else {
        #if DEBUG_LEVEL >= DEBUG_ERROR
          Serial.println(F("Error: Could not take picture!"));         
        #endif
       log_to_thingspeak("Error: Could not take picture!");
      }
  
      lasttime_alert = millis();      
    }
  
    pir_status = digitalRead(PIR_SENSOR_GPIO);  
    if (pir_status) {
      lasttime_pir = millis();
    }
  } while(millis() <= lasttime_pir + ALERT_UPKEEP_INTERVALL*mS_TO_S_FACTOR);
     
}

void check_telegram_commands() {
    camera_fb_t * fb;
    
    #if DEBUG_LEVEL >= DEBUG_VERBOSE
       Serial.println(F(""));
       Serial.println(F("Send a keepalive message to Thingsspeak"));
    #endif
    log_to_thingspeak("Camera is alive");

    #if DEBUG_LEVEL >= DEBUG_DEBUG
      Serial.print(F("Checking for command '/img' in chat_id "));
      Serial.println(ALERT_CHAT_ID);
    #endif
    
    if ( check_for_command_in_chat_id("/img", ALERT_CHAT_ID) ) { 
      log_to_thingspeak("Got a /img request, sending picture");
      #if DEBUG_LEVEL >= DEBUG_INFO
        Serial.print(F("Command /img found for chat_id ")); 
        Serial.println(ALERT_CHAT_ID);
      
        Serial.println(F("Taking and sending picture")); 
      #endif

      fb  = NULL;
      fb = take_picture();
      if ( fb ) {
        send_picture(ALERT_CHAT_ID,fb); 
        esp_camera_fb_return(fb);
      } else {
       #if DEBUG_LEVEL >= DEBUG_ERROR
        Serial.println(F("Error: Could not take picture!")); 
       #endif   
       log_to_thingspeak("Error: Could not take picture!");
      }
    }  
}

void configure_camera() {
    // Configuração da Camera (Tenta 3 vezes de fazer um camera set-up antes de desistir)
    #if DEBUG_LEVEL >= DEBUG_INFO
      Serial.println(F(""));
      Serial.print(F("Setting up Camera ..."));
    #endif

    for (int i = 0; i < 4; i++ ) {
      #if DEBUG_LEVEL >= DEBUG_INFO
        Serial.print(".");
      #endif
      if ( i == 3 ) { 
        #if DEBUG_LEVEL >= DEBUG_WARN
          Serial.println(F(" failure!"));
          Serial.println(F("Third attempt failed: Exiting the sketch."));
        #endif
        
        return; 
        }
      if ( setup_camera() ) {
        #if DEBUG_LEVEL >= DEBUG_INFO
          Serial.println(F(" sucess!"));
        #endif
        
        break; 
        }    
    }  
}

void connect_wifi() {
    // Conectar ao WIFI
    #if DEBUG_LEVEL >= DEBUG_INFO
      Serial.println(F(""));
      Serial.print(F("Connecting Wifi: "));
      Serial.println(WIFI_NAME);
    #endif
  
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_NAME, WIFI_PASSWORD);
  
    while (WiFi.status() != WL_CONNECTED) {
      #if DEBUG_LEVEL >= DEBUG_INFO
        Serial.print(F("."));
      #endif
      delay(100);
    }
  
    #if DEBUG_LEVEL >= DEBUG_INFO
      Serial.println(F(""));
      Serial.println(F("WiFi connected"));
    #endif
    #if DEBUG_LEVEL >= DEBUG_DEBUG
      Serial.print(F("IP address: "));
      Serial.println(WiFi.localIP());
      Serial.print(F("RSSI: "));
      Serial.println(WiFi.RSSI());  
    #endif
}

void setup_pir_sensor() {
  // Configuração do GPIO para o sensor PIR
  #if DEBUG_LEVEL >= DEBUG_DEBUG
    Serial.print(F(""));
    Serial.print(F("Configuring PIR sensor on GPIO No. "));
    Serial.println(PIR_SENSOR_GPIO);
  #endif
  
  pinMode(PIR_SENSOR_GPIO, INPUT);  
}
