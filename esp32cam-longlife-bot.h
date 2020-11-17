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

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "UniversalTelegramBotRZO.h"
#include "settings.h"
#include "esp32cam-longlife-cam.h"

// Functions
void send_picture(String chat_id, camera_fb_t * fb);
void send_text(String chat_id, String text);
int check_for_command_in_chat_id(String cmd, String chat_id);
