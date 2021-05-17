// Boxes for game selection on home screen
int box1_x = 14;
int box1_y = 80;
int box1_width = 50;
int box1_height = 60;
int box2_x = 68;
int box2_y = 80;
int box2_width = 50;
int box2_height = 60;

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
  tft.drawRect(14, 80, 50, 60, ST7735_GREEN);
  tft.drawString("Just", 28, 100, 1);
  tft.drawString("Dance", 25, 110, 1);
  tft.drawRect(68, 80, 50, 60, ST7735_GREEN);
  tft.drawString("Rhythm", 76, 100, 1);
  tft.drawString("Game", 82, 110, 1);
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
  tft.drawString("Get ready to move!", 12, 50, 1);
  delay(2000);
}

void just_dance_end() {
  tft.fillScreen(TFT_BLACK);
  tft.drawString("Total Score:", 25, 30, 2);
  char score[5];
  sprintf(score, "%d", just_dance_total);
  tft.drawString(score, 56, 45, 2);
  tft.setCursor(0, 70, 1); 
  tft.printf(individual_scores);
  tft.drawRect(14, 130, 100, 20, ST7735_GREEN);
  tft.drawString("Home Screen", 30, 136, 1);
  end_screen = true;
}

void select_game(int button) {
  switch(game_state) {
    case 0:
      Serial.println("case 0");
      if (button == 1) {
        game_state = 1;
      }
      break;
    case 1:
      Serial.println("case 1");
      if (button == 0) {
        game_state = 2;
        tft.drawRect(box2_x + 1, box2_y + 1, box2_width - 2, box2_height - 2, BLACK);
        tft.drawRect(box2_x, box2_y, box2_width, box2_height, ST7735_GREEN);
        tft.drawRect(box2_x - 1, box2_y - 1, box2_width + 2, box2_height + 2, BLACK);
        tft.drawRect(box1_x + 1, box1_y + 1, box1_width - 2, box2_height - 2, WHITE);
        tft.drawRect(box1_x, box1_y, box1_width, box1_height, WHITE);
        tft.drawRect(box1_x - 1, box1_y - 1, box1_width + 2, box1_height + 2, WHITE);
        selected_game = 1;
      }
      break;
    case 2:
      Serial.println("case 2");
      if (button == 2) {
        game_state = 5;
      }
      if (button == 1) {
        game_state = 3;
      }
      break;
    case 3:
      Serial.println("case 3");
      if (button == 0) {
        game_state = 4;
        tft.drawRect(box1_x + 1, box1_y + 1, box1_width - 2, box2_height - 2, BLACK);
        tft.drawRect(box1_x, box1_y, box1_width, box2_height, ST7735_GREEN);
        tft.drawRect(box1_x - 1, box1_y - 1, box1_width + 2, box2_height + 2, BLACK);
        tft.drawRect(box2_x + 1, box2_y + 1, box2_width - 2, box2_height - 2, WHITE);
        tft.drawRect(box2_x, box2_y, box2_width, box2_height, WHITE);
        tft.drawRect(box2_x - 1, box2_y -1, box2_width + 2, box2_height + 2, WHITE);
        selected_game = 2;
      }
      break;
    case 4:
      Serial.println("case 4");
      if (button == 2) {
        game_state = 5;
      }
      if (button == 1) {
        game_state = 1;
      }
      break;
    case 5:
      Serial.println("case 5");
      if (button == 0) {
        game_state = 6;
        tft.fillScreen(BLACK);
        tft.drawString("Select a song!", 25, 52, 1);
      }
      break;
    case 6: 
      Serial.println("case 6");
      select_song(button);
      if (button == 2) {
        game_state = 7;
      }
      break;
    case 7:
      Serial.println("case 7");
      if (button == 0) {
        Serial.println("in case 6");
        home_screen = false;
        load_game();
        begin_dance = true;
        begin_rhythm = true;
        song_timer = millis();
        song_state = 1;
        step_num = 0;
        dance_time = time_per_beat * dance_to_play.timing[step_num];
        game_state = 0;
      }
      break;
  }
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
        char output[20];
        sprintf(output, "** %s **", song_names[selected_song]);

        tft.setTextColor(YELLOW, TFT_BLACK);
        if (selected_song == 0) { // Stereo Hearts
          song_to_play = stereo;
          dance_to_play = stereo_advanced;
          tft.drawString(output, 8, 66, 1);
        } else if (selected_song == 1) { // Riptide
          song_to_play = riptide;
          dance_to_play = riptide_basic;
          tft.drawString(output, 24, 66, 1);
        }
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        selected_song += 1;
        selected_song = selected_song % 2;
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
