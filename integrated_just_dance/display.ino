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
  tft.drawString("Starting Just Dance", 7, 35, 1);
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
        tft.drawRect(69, 81, 48, 58, BLACK);
        tft.drawRect(68, 80, 50, 60, ST7735_GREEN);
        tft.drawRect(15, 81, 48, 58, WHITE);
        tft.drawRect(14, 80, 50, 60, WHITE);
        tft.drawRect(13, 79, 52, 62, WHITE);
        selected_game = 1;
      }
      break;
    case 2:
      if (button == 2) {
        game_state = 4;
      }
      if (button == 1) {
        game_state = 3;
      }
      break;
    case 3:
      if (button == 0) {
        game_state = 0;
        tft.drawRect(15, 81, 48, 58, BLACK);
        tft.drawRect(14, 80, 50, 60, ST7735_GREEN);
        tft.drawRect(13, 79, 52, 62, BLACK);
        tft.drawRect(69, 81, 48, 58, WHITE);
        tft.drawRect(68, 80, 50, 60, WHITE);
        tft.drawRect(67, 79, 52, 62, WHITE);
        selected_game = 2;
      }
      break;
    case 4:
      if (button == 0) {
        game_state = 5;
      }
    case 5: 
      home_screen = false;
      end_screen = true;
      load_game();
      begin_dance = true;
      song_timer = millis();
      song_state = 1;
      step_num = 0;
      dance_time = time_per_beat * choreo_timing[step_num];
      game_state = 0;
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
