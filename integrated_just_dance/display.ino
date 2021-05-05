void draw_home_screen() {
  tft.drawString("Who Will Be The", 20, 25, 1);
  tft.drawString("Next Popstar?", 22, 35, 2);
  tft.drawString("Play to find out!", 15, 60, 1);
  tft.drawRect(14, 80, 50, 60, ST7735_GREEN);
  tft.drawString("Just", 28, 100, 1);
  tft.drawString("Dance", 25, 110, 1);
  tft.drawRect(68, 80, 50, 60, ST7735_GREEN);
  tft.drawString("Rhythm", 76, 100, 1);
  tft.drawString("Game", 82, 110, 1);
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
}
