// Boxes for game selection on home screen
int box1_x = 13;
int box1_y = 74;
int box1_width = 50;
int box1_height = 40;
int box2_x = 67;
int box2_y = 74;
int box2_width = 50;
int box2_height = 40;
int box3_x = box1_x;
int box3_y = box1_y + box1_height + 5;
int box3_width = box2_x - box1_x + box2_width;
int box3_height = 20;

void draw_title() {
  tft.drawString("Who Will Be The", 20, 25, 1);
  tft.setTextColor(YELLOW, TFT_BLACK);
  tft.drawString("Next Popstar?", 22, 35, 2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
}

void draw_user_selection_screen() {
  draw_title();
  tft.drawString("Select a username", 15, 60, 1);
  tft.drawString("to start.", 40, 70, 1);
  tft.drawRect(14, 85, 100, 15, LIGHT_GRAY);
  tft.drawRect(13, 84, 102, 17, LIGHT_GRAY);
}

void draw_home_screen() {
  draw_title();
  tft.drawString("Play to find out!", 15, 60, 1);
  tft.drawRect(box1_x, box1_y, box1_width, box1_height, ST7735_GREEN);
  tft.drawString("Just", 27, 86, 1);
  tft.drawString("Dance", 24, 96, 1);
  tft.drawRect(box2_x, box2_y, box2_width, box2_height, ST7735_GREEN);
  tft.drawString("Rhythm", 75, 86, 1);
  tft.drawString("Game", 81, 96, 1);
  tft.drawRect(box3_x, box3_y, box3_width, box3_height, ST7735_GREEN);
  tft.drawString("Karaoke", 45, box3_y + box3_height/2 - 3, 1);
  char username[25];
  sprintf(username, "Username: %s", user);
  Serial.printf("Username... %s \n", username);
  Serial.printf("User... %s \n", user);
  tft.drawString(username, 14, 145, 1);
}

void load_game() {
  tft.fillScreen(TFT_BLACK);
//  tft.drawString("Starting Just Dance", 7, 35, 1);
  tft.drawString("Starting the game...", 7, 35, 1);
  tft.drawString("Get ready to play!", 12, 50, 1);

  delay(2000);
  tft.fillScreen(TFT_BLACK);
  
  if (selected_game == 2) {
    for (int i=0; i<map_to_play.num_notes; i++){ // initialize Note array
      map_notes[i] = Note(&tft, map_to_play.actions[i], map_to_play.indices[i], 1, 0);
    }
    song_timer = millis() + 1500;
  }
  
}

void just_dance_end() {
  ledcWriteTone(AUDIO_PWM, 0);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(WHITE, BLACK);
  tft.setCursor(0, 0);
  tft.setTextSize(1);
  tft.drawString("Total Score:", 25, 10, 2);
  char score[5];
  sprintf(score, "%d", just_dance_total);
  tft.drawString(score, 56, 25, 2);
  tft.setCursor(0, 50, 1); 
  tft.printf(individual_scores);
  tft.fillRect(0, 124, 114, 60, BLACK); 
  tft.drawRect(14, 130, 100, 20, ST7735_GREEN);
  tft.drawString("Home Screen", 30, 136, 1);
  end_screen = true;
}

void rhythm_end() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(WHITE, BLACK);
  tft.setCursor(0, 0);
  tft.setTextSize(1);
  tft.drawString("Total Score:", 25, 30, 2);
  char score[8];
  sprintf(score, "%d", rhythm_score);
  tft.drawString(score, 50, 45, 2);

  tft.drawRect(14, 130, 100, 20, ST7735_GREEN);
  tft.drawString("Home Screen", 30, 136, 1);
  end_screen = true;
}

void select_game(int button) {
  switch(game_state) {
    case 0:
      if (button == 1) {
        game_state = 1;
      }
      break;
    case 1:
      if (button == 0) {
        game_state = 2;
        draw_erase_box(box3_x, box3_y, box3_width, box3_height, box1_x, box1_y, box1_width, box1_height);
        selected_game = 1;
      }
      break;
    case 2:
      if (button == 2) {
        game_state = 7;
      }
      if (button == 1) {
        game_state = 3;
      }
      break;
    case 3:
      if (button == 0) {
        game_state = 4;
        draw_erase_box(box1_x, box1_y, box1_width, box1_height, box2_x, box2_y, box2_width, box2_height);
        selected_game = 2;
      }
      break;
    case 4:
      if (button == 2) {
        game_state = 7;
      }
      if (button == 1) {
        game_state = 5;
      }
      break;
    case 5:
      if (button == 0) {
        game_state = 6;
        draw_erase_box(box2_x, box2_y, box2_width, box2_height, box3_x, box3_y, box3_width, box3_height);
        selected_game = 3;
      }
      break;
    case 6:
       if (button == 2) {
        game_state = 7;
       } 
       if (button == 1) {
        game_state = 1;
       }
       break;
    case 7:
      if (button == 0) {
        game_state = 8;
        tft.fillScreen(BLACK);
        tft.drawString("Select a song!", 25, 52, 1);
      }
      break;
    case 8: 
      select_song(button);
      if (button == 2) {
        game_state = 9;
      }
      break;
    case 9:
      if (button == 0) {
        home_screen = false;
        load_game();
        begin_dance = true;
        begin_rhythm = true;
        begin_karaoke = true;
        if (selected_game == 2) {
          song_timer = millis() + 1500;
        } else {
          song_timer = millis();
        }
        song_state = 1;
        step_num = 0;
        time_per_beat = (60 * 1000) / tempo;
        dance_time = time_per_beat * dance_to_play.timing[step_num];
        game_state = 0;
//        if (selected_song == 3) {
//          Serial.println("song 3");
//          readFile(SD, "/stereo.txt");
//          double_extractor(message_buffer, stereo_long.notes, ',');
//          song_to_play = stereo_long;
//        } else if (selected_song == 4) {
//          readFile(SD, "/riptide.txt");
//          double_extractor(message_buffer, riptide_long.notes, ',');
//          song_to_play = riptide_long;
//        }
      }
      break;
  }
}

// Erase box b1, draw box b2
void draw_erase_box(int b1x, int b1y, int b1w, int b1h, int b2x, int b2y, int b2w, int b2h) {
  tft.drawRect(b1x + 1, b1y + 1, b1w - 2, b1h - 2, BLACK);
  tft.drawRect(b1x, b1y, b1w, b1h, ST7735_GREEN);
  tft.drawRect(b1x - 1, b1y - 1, b1w + 2, b1h + 2, BLACK);
  tft.drawRect(b2x + 1, b2y + 1, b2w - 2, b2h - 2, WHITE);
  tft.drawRect(b2x, b2y, b2w, b2h, WHITE);
  tft.drawRect(b2x - 1, b2y - 1, b2w + 2, b2h + 2, WHITE);
}

void select_song(int button) {
  switch(song_pick_state) {
    case 0:
      if (button == 1) {
        song_pick_state = 1;
      }
      break;
    case 1:
      if (button == 0) {
        song_pick_state = 0;
        tft.fillRect(0, 66, 128, 10, BLACK);
        char output[50];
        sprintf(output, "** %s **", song_names[selected_song]);

        tft.setTextColor(YELLOW, TFT_BLACK);
        if (selected_song == 0) { // Stereo Hearts
          Serial.println("song 1");
          tempo = 90;
          song_to_play = stereo;
          dance_to_play = stereo_easy;
          map_to_play = stereo_map;
          tft.drawString(output, 8, 66, 1);
        } else if (selected_song == 1) { // Riptide
          Serial.println("song 2");
          tempo = 102;
          song_to_play = riptide;
          dance_to_play = riptide_basic;
          map_to_play = riptide_map;
          tft.drawString(output, 26, 66, 1);
//        } else if (selected_song == 2) { // Long Stereo Hearts
//          tempo = 90;
//          dance_to_play = stereo_easy;
//          map_to_play = stereo_map;
//          tft.drawString(output, 4, 66, 1);
//        } else if (selected_song == 3) { // Long Riptide 
//          tempo = 100;
//          dance_to_play = riptide_easy;
//          map_to_play = riptide_map;
//          tft.drawString(output, 22, 66, 1);
        } else if (selected_song == 2) { // Shake it Off
          tempo = 160;
          song_to_play = shake;
          dance_to_play = shake_easy;
          map_to_play = shake_map;
          tft.drawString(output, 11, 66, 1);
        } else if (selected_song == 3) { // Havana
          tempo = 105;
          song_to_play = havana;
          dance_to_play = havana_easy;
          map_to_play = havana_map;
          tft.drawString(output, 27, 66, 1);
        }
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        selected_song += 1;
        selected_song = selected_song % 4;
      }
      break;
  }
}

void finish_game(int button) {
  switch(finish_state) {
    case 0:
      if (button == 1) {
        finish_state = 1;
      } 
      break;
    case 1:
      if (button == 0) {
        finish_state = 2;
        tft.drawRect(15, 131, 98, 18, WHITE);
        tft.drawRect(14, 130, 100, 20, WHITE);
        tft.drawRect(13, 129, 102, 22, WHITE);
      }
      break;
    case 2:
      if (button == 2) {
        tft.fillScreen(BLACK);
        draw_home_screen();
        memset(individual_scores, 0, 500);
        just_dance_total = 0;
        home_screen = true;
        end_screen = false;
        finish_state = 0;
      }
      break;
  }
}
