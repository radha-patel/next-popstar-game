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

void sprinkler() {
  switch(sprinkler_state) {
    case 0:
      tft.fillRect(0, 0, 128 * move_iter / choreo_timing[step_num], 20, WHITE);
      tft.fillRect(0, 20, 128, 140, CYAN);
      sprinkler_state = 1;
      break;
    case 1:
      if (z < -2) {
        sprinkler_state = 2;
        Serial.println("Sprinkler: stage 1");
      }
      break;
    case 2:
      if (z > 2) {
        sprinkler_state = 3;
        Serial.println("Sprinkler: stage 2");
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
      tft.fillRect(0, 0, 128 * move_iter / choreo_timing[step_num], 20, WHITE);
      tft.fillRect(0, 20, 128, 140, YELLOW);
      arm_cross_state = 1;
      break;
    case 1:
      if (z < -3) {
        arm_cross_state = 2;
        Serial.println("Arm Cross: stage 1");
      }
      break;
    case 2:
      if (z > 3) {
        arm_cross_state = 3;
        Serial.println("Arm Cross: stage 2");
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
      tft.fillRect(0, 0, 128 * move_iter / choreo_timing[step_num], 20, WHITE);
      tft.fillRect(0, 20, 128, 140, MAROON);
      disco_state = 1;
      break;
    case 1:
      if (x > 1) {
        disco_state = 2;
        Serial.println("Disco: stage 1");
      }
      break;
    case 2:
      if (y > 5) {
        disco_state = 3;
        Serial.println("Disco: stage 2");
      }
      break;
    case 3:
      if (x < -1) {
        disco_state = 4;
        Serial.println("Disco: stage 3");
      }
      break;
    case 4:
      if (y < 3) {
        disco_state = 5;
        Serial.println("Disco: stage 4");
      }
      break;
    case 5:
      disco_state = 0;
      move_iter += 1;
      break;
  }
}
