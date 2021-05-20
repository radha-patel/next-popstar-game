
//function used to record audio at sample rate for a fixed nmber of samples
void record_audio(int * index) {
  int sample_num = 0;    // counter for samples
  int enc_index = 0;  // index counter for encoded samples
  float time_between_samples = 1000000 / SAMPLE_FREQ;
  int value = 0;
  char raw_samples[3];   // 8-bit raw sample data array
  memset(speech_data, 0, sizeof(speech_data));
  char holder[5] = {0};
  Serial.println("starting");
  uint32_t text_index = enc_index;
  uint32_t start = millis();
  time_since_sample = micros();
  while (sample_num < NUM_SAMPLES) { //read in NUM_SAMPLES worth of audio data
//    button_state = digitalRead(PIN_1);
//    if (button_state) break;
    value = analogRead(AUDIO_IN);  //make measurement
    raw_samples[sample_num % 3] = mulaw_encode(value - 1551); //remove 1.25V offset (from 12 bit reading)
    sample_num++;
    if (sample_num % 3 == 0) {
      base64_encode(holder, raw_samples, 3);
      strncat(speech_data + text_index, holder, 4);
      text_index += 4;
    }
    // wait till next time to read
    while (micros() - time_since_sample <= time_between_samples); //wait...
    time_since_sample = micros();
  }
  Serial.println(millis() - start);
  int len = strlen(speech_data);
  char file_name[50] = "";
  sprintf(file_name, "/audio%i.txt", *index);
  Serial.println(len);
  writeFile(SD, file_name, speech_data);
  Serial.println("out");
  *index = *index + 1;
}

void post_audio(char * message, int message_len) {
  Serial.println("Posting to Server:");      
  sprintf(request, "POST http://608dev-2.net/sandbox/sc/team64/karaoke_server_withdb.py HTTP/1.1\r\n");
  sprintf(request + strlen(request), "Host: %s\r\n", host);
  strcat(request, "Content-Type: application/x-www-form-urlencoded\r\n");
  sprintf(request + strlen(request), "Content-Length: %d\r\n\r\nuser=test&audio=", message_len);
  strcat(request, message);
//  Serial.println(request);
  do_http_request("608dev-2.net", request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
}

void get_fft(char * user, float * score, char * song_title) {
  sprintf(request, "GET /sandbox/sc/team64/karaoke_server_withdb.py?song=%s&user=%s HTTP/1.1\r\n", song_title, user);
  strcat(request, "Host: 608dev-2.net\r\n");
  strcat(request, "\r\n"); //new line from header to body

  Serial.println(request);

  do_http_request("608dev-2.net", request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
  *score = atof(response);
}
