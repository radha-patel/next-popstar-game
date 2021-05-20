#include <SPI.h>
#include <mpu6050_esp32.h>
#include<math.h>
#include<string.h>
#include <WiFi.h>
#include <TFT_eSPI.h>
#include "FS.h"
#include "SD.h"

TaskHandle_t Task1;
TaskHandle_t Task2;

TFT_eSPI tft = TFT_eSPI();

const int DELAY = 1000;
const int SAMPLE_FREQ = 10000;                          // Hz, telephone sample rate
const int SAMPLE_DURATION = 1;                        // duration of fixed sampling (seconds)
const int NUM_SAMPLES = SAMPLE_FREQ * SAMPLE_DURATION;  // number of of samples
const int ENC_LEN = (NUM_SAMPLES + 2 - ((NUM_SAMPLES + 2) % 3)) / 3 * 4;  // Encoded length of clip

/* CONSTANTS */
//Prefix to POST request:
const int AUDIO_IN = A0; //pin where microphone is connected
const char API_KEY[] = "AIzaSyCwyynsePu7xijUYTOgR7NdVqxH2FAG9DQ"; //don't change this


const uint8_t PIN_1 = 3; //button 1
const uint8_t PIN_2 = 0; //button 2


/* Global variables*/
uint8_t button_state; //used for containing button state and detecting edges
int old_button_state; //used for detecting button edges
uint32_t time_since_sample;      // used for microsecond timing


char speech_data[ENC_LEN + 200] = {0}; //global used for collecting speech data

//Code for sending POST request at the end

char network[] = "MIT";  //SSID for 6.08 Lab
char password[] = ""; //Password for 6.08 Lab
char request[15000];

char host[] = "608dev-2.net";

uint8_t old_val;
uint32_t timer;

char message_buffer[15000] = "";

int lyric_index = 0;

//Some constants and some resources:
const int RESPONSE_TIMEOUT = 100000000000; //ms to wait for response from host
const uint16_t OUT_BUFFER_SIZE = 1000; //size of buffer to hold HTTP response
char old_response[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP request
char response[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP request

int file_count = 0;
int record_state = 0;
//int song_length = 109335; // stereo as example, 656 * 166.67
//int song_length = 3000; // test length (about 4 measures of stereo)
int song_length = 21334 * 2; // 8 measures is 21334

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

void get_fft(char * user) {
  sprintf(request, "GET /sandbox/sc/team64/karaoke_server_withdb.py?song=stereo&user=%s HTTP/1.1\r\n", user);
  strcat(request, "Host: 608dev-2.net\r\n");
  strcat(request, "\r\n"); //new line from header to body

  do_http_request("608dev-2.net", request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
}
struct Riff {
  double notes[1500]; //the notes (array of doubles containing frequencies in Hz. I used https://pages.mtu.edu/~suits/notefreqs.html
  int length; //number of notes (essentially length of array.
  float note_period; //the timing of each note in milliseconds (take bpm, scale appropriately for note (sixteenth note would be 4 since four per quarter note) then
};

Riff song_to_play = {{}, 656, 166.67};
char *lyrics_stereo[7] = {"My heart's a stereo", "It beats for your, so listen close", "Hear my thoughts in every note", "Make me your radio", "Turn me up when you feel low", "This melody was meant for you", "Just sing along to my stereo..." };
const int BUTTON_PIN = 3;
int song_state = 0;
int song_index = 0;
uint32_t song_timer;

uint8_t AUDIO_TRANSDUCER = 26;
uint8_t AUDIO_PWM = 1;

//global variables to help your code remember what the last note was to prevent double-playing a note which can cause audible clicking
float new_note = 0;
float old_note = 0;
char freq_buffer[100];

int game_end = 0;

void setup() {
  Serial.begin(115200);               // Set up serial port
  delay(100); //wait a bit (100 ms)
  pinMode(PIN_1, INPUT_PULLUP);
  pinMode(PIN_2, INPUT_PULLUP);

  tft.init();
  tft.setRotation(2);
//  primary_timer = millis();
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);


  //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
                    Task1code,   /* Task function. */
                    "Task1",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task1,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */                  
  delay(500); 

  //create a task that will be executed in the Task2code() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
                    Task2code,   /* Task function. */
                    "Task2",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task2,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 1 */
    delay(500); 



  if(!SD.begin()){
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
    Serial.println("No SD card attached");
    return;
  }

    Serial.print("SD Card Type: ");
    if(cardType == CARD_MMC){
        Serial.println("MMC");
    } else if(cardType == CARD_SD){
        Serial.println("SDSC");
    } else if(cardType == CARD_SDHC){
        Serial.println("SDHC");
    } else {
        Serial.println("UNKNOWN");
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);

    Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
    Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));
WiFi.begin(network, password); //attempt to connect to wifi
  uint8_t count = 0; //count used for Wifi check times
  Serial.print("Attempting to connect to ");
  Serial.println(network);
  while (WiFi.status() != WL_CONNECTED && count < 12) {
    delay(500);
    Serial.print(".");
    count++;
  }
  delay(2000);
  if (WiFi.isConnected()) { //if we connected then print our IP, Mac, and SSID we're on
    Serial.println("CONNECTED!");
    Serial.printf("%d:%d:%d:%d (%s) (%s)\n", WiFi.localIP()[3], WiFi.localIP()[2],
                  WiFi.localIP()[1], WiFi.localIP()[0],
                  WiFi.macAddress().c_str() , WiFi.SSID().c_str());    delay(500);
  } else { //if we failed to connect just Try again.
    Serial.println("Failed to Connect :/  Going to restart");
    Serial.println(WiFi.status());
    ESP.restart(); // restart the ESP (proper way)
  }
  delay(50);
  Wire.begin();
  delay(50); 
    readFile(SD, "/stereo.txt", message_buffer);
    Serial.println(message_buffer);
    listDir(SD, "/", 0);
    Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
    Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));
    double_extractor(message_buffer, song_to_play.notes, ',');

    // set up pins
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(AUDIO_TRANSDUCER, OUTPUT);
  
    //set up AUDIO_PWM_1 which we will control in this lab for music:
    ledcSetup(AUDIO_PWM, 0, 12);//12 bits of PWM precision
    ledcWrite(AUDIO_PWM, 0); //0 is a 0% duty cycle for the NFET
    ledcAttachPin(AUDIO_TRANSDUCER, AUDIO_PWM);


  song_timer = millis();
  song_state = 0;
  
  timer = millis();
  old_val = digitalRead(PIN_1);

  
}

//Task1code: blinks an LED every 1000 ms
void Task1code( void * pvParameters ){
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());

  for(;;){
  if (!digitalRead(BUTTON_PIN)) {
    song_state = 1;
    song_timer = millis();
  }
  // music playing section
  if (song_state){
   if (song_index == 0) {
      Serial.println("Start music!");
      ledcWriteTone(AUDIO_PWM, song_to_play.notes[song_index]);
      song_index++;
      } 
      if (millis() - song_timer >= song_index * song_to_play.note_period) { // time to switch to the next note
        if (song_index == song_to_play.length) { // end of song
          ledcWriteTone(AUDIO_PWM, 0);
          song_state = 0; 
          song_index = 0;
          game_end = true;
        } else if (song_to_play.notes[song_index] != song_to_play.notes[song_index-1]) { // note change
          ledcWriteTone(AUDIO_PWM, song_to_play.notes[song_index]);
          song_index ++;
        } else song_index++; // otherwise increment index 
        if ((song_index-1)%16 == 0) {
          Serial.println(song_index);
          tft.fillScreen(TFT_BLACK);
          tft.setCursor(17, 89, 1);
          tft.println(lyrics_stereo[lyric_index]);
          if (lyric_index < 7) {
            lyric_index++;
  
          }
        }
      }
  }
  delay(50);
  } 
}

void play_lyrics() {
  if (song_index == 0) {
      Serial.println("Start music!");
      ledcWriteTone(AUDIO_PWM, song_to_play.notes[song_index]);
      song_index++;
      } 
      if (millis() - song_timer >= song_index * song_to_play.note_period) { // time to switch to the next note
        if (song_index == song_to_play.length) { // end of song
          ledcWriteTone(AUDIO_PWM, 0);
          song_state = 0; 
          song_index = 0;
          game_end = true;
        } else if (song_to_play.notes[song_index] != song_to_play.notes[song_index-1]) { // note change
          ledcWriteTone(AUDIO_PWM, song_to_play.notes[song_index]);
          song_index ++;
        } else song_index++; // otherwise increment index 
        if ((song_index-1)%16 == 0) {
          Serial.println(song_index);
          tft.fillScreen(TFT_BLACK);
          tft.setCursor(17, 89, 1);
          tft.println(lyrics_stereo[lyric_index]);
          if (lyric_index < 7) {
            lyric_index++;
  
          }
        }
      }
    
  }

//Task2code: blinks an LED every 700 ms
void Task2code( void * pvParameters ){
  Serial.print("Task2 running on core ");
  Serial.println(xPortGetCoreID());
  for (;;) {
    button_state = digitalRead(PIN_1);
  if (!button_state) record_state = 1;
  if (record_state == 1) { // recording state
    timer = millis();
    file_count = 0;
    // do the timed recording here
    while (millis() - timer < song_length) {
      record_audio(&file_count);
      Serial.print("File index:");
      Serial.println(file_count);
    }
    listDir(SD, "/", 0);
    record_state = 2; // switch to send state
  } else if (record_state == 2) {
    for (int i = 0; i < file_count; i++) {
      // send each of the stored encoded files on the SD card to the server
      char file_name[50] = "";
      sprintf(file_name, "/audio%i.txt", i);
      readFile(SD, file_name, message_buffer);
      int message_len = strlen(message_buffer);
      message_len = message_len + 16;
      post_audio(message_buffer, message_len);
      delay(50);
    }
    get_fft("test");
    record_state = 0; // set back to idle state
  }
//  if (!button_state && button_state != old_button_state) {
//    Serial.println("listening...");
//    record_audio();
//    Serial.println("sending...");
//    readFile(SD, "/test_sample.txt", message_buffer);
//    Serial.println(strlen(message_buffer));
//    Serial.println("sending data");
//    post_audio(message_buffer);
//    }
  old_button_state = button_state;
  delay(50);
  }

  }

//main body of code
void loop() {
  
}

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


int8_t mulaw_encode(int16_t sample) {
  //paste the fast one here.
   const uint16_t MULAW_MAX = 0x1FFF;
   const uint16_t MULAW_BIAS = 33;
   uint16_t mask = 0x1000;
   uint8_t sign = 0;
   uint8_t position = 12;
   uint8_t lsb = 0;
   if (sample < 0)
   {
      sample = -sample;
      sign = 0x80;
   }
   sample += MULAW_BIAS;
   if (sample > MULAW_MAX)
   {
      sample = MULAW_MAX;
   }
   for (; ((sample & mask) != mask && position >= 5); mask >>= 1, position--)
        ;
   lsb = (sample >> (position - 4)) & 0x0f;
   return (~(sign | ((position - 5) << 4) | lsb));
}


// POST request functions

/*----------------------------------
  char_append Function:
  Arguments:
     char* buff: pointer to character array which we will append a
     char c:
     uint16_t buff_size: size of buffer buff

  Return value:
     boolean: True if character appended, False if not appended (indicating buffer full)
*/
uint8_t char_append(char* buff, char c, uint16_t buff_size) {
  int len = strlen(buff);
  if (len > buff_size) return false;
  buff[len] = c;
  buff[len + 1] = '\0';
  return true;
}

/*----------------------------------
   do_http_request Function:
   Arguments:
      char* host: null-terminated char-array containing host to connect to
      char* request: null-terminated char-arry containing properly formatted HTTP request
      char* response: char-array used as output for function to contain response
      uint16_t response_size: size of response buffer (in bytes)
      uint16_t response_timeout: duration we'll wait (in ms) for a response from server
      uint8_t serial: used for printing debug information to terminal (true prints, false doesn't)
   Return value:
      void (none)
*/
void do_http_request(char* host, char* request, char* response, uint16_t response_size, uint16_t response_timeout, uint8_t serial) {
  WiFiClient client; //instantiate a client object
  if (client.connect(host, 80)) { //try to connect to host on port 80
    if (serial) Serial.print(request);//Can do one-line if statements in C without curly braces
    client.print(request);
    memset(response, 0, response_size); //Null out (0 is the value of the null terminator '\0') entire buffer
    uint32_t count = millis();
    while (client.connected()) { //while we remain connected read out data coming back
      client.readBytesUntil('\n', response, response_size);
      if (serial) Serial.println(response);
      if (strcmp(response, "\r") == 0) { //found a blank line!
        break;
      }
      memset(response, 0, response_size);
      if (millis() - count > response_timeout) break;
    }
    memset(response, 0, response_size);
    count = millis();
    while (client.available()) { //read out remaining text (body of response)
      char_append(response, client.read(), OUT_BUFFER_SIZE);
    }
    if (serial) Serial.println(response);
    client.stop();
    if (serial) Serial.println("-----------");
  } else {
    if (serial) Serial.println("connection failed :/");
    if (serial) Serial.println("wait 0.5 sec...");
    client.stop();
  }
}
