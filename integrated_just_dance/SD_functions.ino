void readFile(fs::FS &fs, const char * path){
    int offset = 0;
    Serial.printf("Reading file: %s\n", path);

    File file = fs.open(path);
    if(!file){
        Serial.println("Failed to open file for reading");
        return;
    }

    Serial.print("Read from file: ");
//    Serial.println("debug 1");
    while(file.available()){
//      Serial.println("debug 2");
      char letter = file.read();
//      Serial.println("debug 3");
      Serial.print(letter);
//      Serial.println("debug 4");
      sprintf(message_buffer + offset, "%c", letter);
//      Serial.println("debug 5");
      offset++;
    }
    Serial.println(message_buffer);
    file.close();
}

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("Failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.name(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

void double_extractor(char* data_array, double* output_values, char delimiter){
    //your code here
      int count = 0;
      int help = 0;
      int output_buffer[20];
      char del[10] = {delimiter};
      char * token;
      token = strtok (data_array,del);
      while (token != NULL)
      {
        help = atof(token);
        output_values[count] = help;
        count++;
        token = strtok (NULL, del);
      }
}
