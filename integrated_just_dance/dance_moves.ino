void punch() {
  switch(punch_state) {
    case 0:
      tft.fillRect(0, 0, 128 * move_iter / dance_to_play.counts[step_num], 20, WHITE);
      tft.fillRect(0, 20, 128, 140, GREEN);
      tft.setTextColor(BLACK, GREEN);
      tft.setCursor(20, 50);
      tft.setTextSize(3);
      tft.println("Punch");
//      tft.drawString("Punch", 50, 50, 2);
      punch_state = 1;
      break;
    case 1:
      if (z < - 3) {
        punch_state = 2;
      }
      break;
    case 2: 
      if (z > 3) {
        punch_state = 3;
      }
      break;
    case 3: 
      punch_state = 0;
      move_iter += 1;
  }
}

void hand_roll() {
  switch(hand_roll_state) {
    case 0:
      tft.fillRect(0, 0, 128 * move_iter / dance_to_play.counts[step_num], 20, WHITE);
      tft.fillRect(0, 20, 128, 140, PURPLE);
      tft.setTextColor(BLACK, PURPLE);
      tft.setCursor(32, 50);
      tft.setTextSize(3);
      tft.println("Roll");
      hand_roll_state = 1;
      break;
    case 1: 
      if (y > 3) {
        hand_roll_state = 2;
      }
      break;
    case 2:
      if (z < 2) {
        hand_roll_state = 3;
      }
      break;
    case 3:
      if (y < -3) {
        hand_roll_state = 4;
      }
      break;
    case 4:
      if (z > 11) {
        hand_roll_state = 5;
      }
      break;
    case 5:
      hand_roll_state = 0;
      move_iter += 1;
  }
}

void wave() {
  switch(wave_state) {
    case 0:
      tft.fillRect(0, 0, 128 * move_iter / dance_to_play.counts[step_num], 20, WHITE);
      tft.fillRect(0, 20, 128, 140, BLUE);
      tft.setTextColor(BLACK, BLUE);
      tft.setCursor(32, 50);
      tft.setTextSize(3);
      tft.println("Wave");
      wave_state = 1;
      break;
    case 1:
      if (y < -9) {
        wave_state = 2;
      }
      break;
    case 2:
      if (y > 11) {
        wave_state = 3;
      }
      break;
    case 3:
      wave_state = 0;
      move_iter += 1;
  }
}

void bounce() {
  switch(bounce_state) {
    case 0:
      tft.fillRect(0, 0, 128 * move_iter / dance_to_play.counts[step_num], 20, WHITE);
      tft.fillRect(0, 20, 128, 140, MAGENTA);
      tft.setTextColor(BLACK, MAGENTA);
      tft.setCursor(14, 50);
      tft.setTextSize(3);
      tft.println("Bounce");
      bounce_state = 1;
      break;
    case 1:
      if (z > 13) {
        bounce_state = 2;
      }
      break;
    case 2:
      if (z < 4) {
        bounce_state = 3;
      }
      break;
    case 3:
      bounce_state = 0;
      move_iter += 1;
  }
}

void sprinkler() {
  switch(sprinkler_state) {
    case 0:
      tft.fillRect(0, 0, 128 * move_iter / dance_to_play.counts[step_num], 20, WHITE);
      tft.fillRect(0, 20, 128, 140, CYAN);
      tft.setTextColor(BLACK, CYAN);
      tft.setCursor(12, 50);
      tft.setTextSize(2);
      tft.println("Sprinkler");
      sprinkler_state = 1;
      break;
    case 1:
      if (z < -2) {
        sprinkler_state = 2;
      }
      break;
    case 2:
      if (z > 2) {
        sprinkler_state = 3;
      }
      break;
    case 3:
      sprinkler_state = 0;
      move_iter += 1;
      break;
  }
}

void arm_cross() {
  switch(arm_cross_state) {
    case 0:
      tft.fillRect(0, 0, 128 * move_iter / dance_to_play.counts[step_num], 20, WHITE);
      tft.fillRect(0, 20, 128, 140, YELLOW);
      tft.setTextColor(BLACK, YELLOW);
      tft.setCursor(12, 50);
      tft.setTextSize(2);
      tft.println("Arm Cross");
      arm_cross_state = 1;
      break;
    case 1:
      if (z < -3) {
        arm_cross_state = 2;
      }
      break;
    case 2:
      if (z > 3) {
        arm_cross_state = 3;
      }
      break;
    case 3:
      arm_cross_state = 0;
      move_iter += 1;
      break;
  }
}

void disco() {
  switch(disco_state) {
    case 0:
      tft.fillRect(0, 0, 128 * move_iter / dance_to_play.counts[step_num], 20, WHITE);
      tft.fillRect(0, 20, 128, 140, MAROON);
      tft.setTextColor(BLACK, MAROON);
      tft.setCursor(20, 50);
      tft.setTextSize(3);
      tft.println("Disco");
      disco_state = 1;
      break;
    case 1:
      if (x > 1) {
        disco_state = 2;
      }
      break;
    case 2:
      if (y > 5) {
        disco_state = 3;
      }
      break;
    case 3:
      if (x < -1) {
        disco_state = 4;
      }
      break;
    case 4:
      if (y < 3) {
        disco_state = 5;
      }
      break;
    case 5:
      disco_state = 0;
      move_iter += 1;
      break;
  }
}

void clap() {
  switch(clap_state) {
    case 0:
      tft.fillRect(0, 0, 128 * move_iter / dance_to_play.counts[step_num], 20, WHITE);
      tft.fillRect(0, 20, 128, 140, LIGHT_GRAY);
      tft.setTextColor(BLACK, LIGHT_GRAY);
      tft.setCursor(28, 50);
      tft.setTextSize(3);
      tft.println("Clap");
      clap_state = 1;
      break;
    case 1:
      if (z > 8) {
        clap_state = 2;
      }
      break;
    case 2:
      if (z < 0) {
        clap_state = 3;
      }
      break;
    case 3:
      clap_state = 0;
      move_iter += 1;
      break;
  }
}

void fist_pump() {
  switch(fist_pump_state) {
    case 0:
      tft.fillRect(0, 0, 128 * move_iter / dance_to_play.counts[step_num], 20, WHITE);
      tft.fillRect(0, 20, 128, 140, LIGHT_GRAY);
      tft.setTextColor(BLACK, LIGHT_GRAY);
      tft.setCursor(12, 50);
      tft.setTextSize(2);
      tft.println("Fist Pump");
      fist_pump_state = 1;
      break;
    case 1:
      if (z < -12) {
        fist_pump_state = 2;
      }
      break;
    case 2:
      if (z > -5) {
        fist_pump_state = 3;
      }
      break;
    case 3:
      fist_pump_state = 0;
      move_iter += 1;
      break;
  }
}

void arm_press() {
  switch(arm_press_state) {
    case 0:
      tft.fillRect(0, 0, 128 * move_iter / dance_to_play.counts[step_num], 20, WHITE);
      tft.fillRect(0, 20, 128, 140, LIGHT_GRAY);
      tft.setTextColor(BLACK, LIGHT_GRAY);
      tft.setCursor(12, 50);
      tft.setTextSize(2);
      tft.println("Arm Press");
      arm_press_state = 1;
      break;
    case 1:
      if (z > 18) {
        arm_press_state = 2;
      }
      break;
    case 2:
      if (z < 4) {
        arm_press_state = 3;
      }
      break;
    case 3:
      arm_press_state = 0;
      move_iter += 1;
      break;
  }
}
