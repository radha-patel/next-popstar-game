void punch() {
  switch(punch_state) {
    case 0:
      tft.fillRect(0, 0, 128 * move_iter / choreo_timing[step_num], 20, WHITE);
      tft.fillRect(0, 20, 128, 140, GREEN);
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
      tft.fillRect(0, 0, 128 * move_iter / choreo_timing[step_num], 20, WHITE);
      tft.fillRect(0, 20, 128, 140, PURPLE);
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
      tft.fillRect(0, 0, 128 * move_iter / choreo_timing[step_num], 20, WHITE);
      tft.fillRect(0, 20, 128, 140, BLUE);
      wave_state = 1;
      break;
    case 1:
      if (y < -9) {
        wave_state = 2;
        Serial.println("Wave: stage 1");
      }
      break;
    case 2:
      if (y > 11) {
        wave_state = 3;
        Serial.println("Wave: state 2");
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
      tft.fillRect(0, 0, 128 * move_iter / choreo_timing[step_num], 20, WHITE);
      tft.fillRect(0, 20, 128, 140, MAGENTA);
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
