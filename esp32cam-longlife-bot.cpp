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

#include "esp32cam-longlife-bot.h"

WiFiClientSecure client_bot;
UniversalTelegramBot bot(BOTtoken, client_bot);


size_t fb_length;
uint8_t* fb_buffer;
int currentByte;


bool isMoreDataAvailable() {
  return (fb_length - currentByte);
}

uint8_t photoNextByte() {
  currentByte++;
  return (fb_buffer[currentByte - 1]);
}

void send_picture(String chat_id, camera_fb_t * fb)
{
  currentByte = 0;
  fb_length = fb->len;
  fb_buffer = fb->buf;

  String response = bot.sendPhotoByBinary(chat_id, "image/jpeg", fb->len, isMoreDataAvailable, photoNextByte, nullptr, nullptr);
}

void send_text(String chat_id, String text) {
  bot.sendSimpleMessage(ALERT_CHAT_ID,text,"Markdown");
}

int check_for_command_in_chat_id(String cmd, String chat_id) {
    int found = 0;

    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      #if DEBUG_LEVEL >= DEBUG_DEBUG
        Serial.print(F("Got "));
        Serial.print(numNewMessages);
        Serial.println(F(" new Messages"));
      #endif
      
      for (int i = 0; i < numNewMessages; i++) {
        #if DEBUG_LEVEL >= DEBUG_VERBOSE
          Serial.print(F("Reading message from chat "));
          Serial.print(bot.messages[i].chat_title);
          Serial.print(F(" ("));
          Serial.print(bot.messages[i].chat_id);
          Serial.print(F(") of type "));
          Serial.print(bot.messages[i].type);
          Serial.print(F(" with content '"));
          Serial.print(bot.messages[i].text);
          Serial.println(F("'"));
        #endif        
     
        if (bot.messages[i].type == "message" and \
            bot.messages[i].text == cmd and \
            bot.messages[i].chat_id == chat_id) { 
              
          #if DEBUG_LEVEL >= DEBUG_DEBUG
            Serial.print(F("Found request '"));
            Serial.print(bot.messages[i].text);
            Serial.print(F("' for image in chat_id '")); 
            Serial.print(chat_id);
            Serial.println(F("' "));
          #endif

          found = 1;
        }
      }

      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    
  return found;
}
