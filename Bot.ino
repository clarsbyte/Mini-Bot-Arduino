#include "U8glib.h"

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_DEV_0 | U8G_I2C_OPT_NO_ACK | U8G_I2C_OPT_FAST); // Fast I2C / TWI

const int ButtonRight = 8;
const int ButtonLeft = 9;
const int ButtonPress = 10;

int state = 1; 
int screen = 0; // 0 - Chat | 1 - Choice
bool buttonWasPressed = false;
bool leftWasPressed = false;
bool rightWasPressed = false;

String inputText = "";
String choices[2];
String question = "";
char choicesChar[2][15];
char questionChar[50];

void setup() {
  Serial.begin(115200);
  u8g.setColorIndex(1);
  pinMode(ButtonRight, INPUT_PULLUP);
  pinMode(ButtonLeft, INPUT_PULLUP);
  pinMode(ButtonPress, INPUT_PULLUP);
}

void Question(const char* q) {
  u8g.setFont(u8g_font_fur14);
  char lines[4][13] = {""}; // Array
  int lineIndex = 0; // 1,2,3,4 lines
  int lenNow = 0;

  const char* wordStart = q;
  const char* p = q; // Pointer - like links


  while (*p) {
    if (*p == ' ' || *(p + 1) == '\0') { // Kalo None \0
      int wordLen = p - wordStart + (*(p + 1) == '\0' ? 1 : 0);
      if (lenNow + wordLen > 12) {
        lines[lineIndex][lenNow] = '\0';
        lineIndex++;
        lenNow = 0;
        if (lineIndex >= 4) break;
      }

      if (lineIndex < 4 && lenNow + wordLen <= 12) {
        strncat(lines[lineIndex], wordStart, wordLen);
        lenNow += wordLen;
        if (*(p + 1) != '\0') {
          strncat(lines[lineIndex], " ", 1);
          lenNow++;
        }
      }

      wordStart = p + 1;
    }


    p++;
  }



  for (int i = 0; i < 4; i++) {
    if (strlen(lines[i]) > 0) {
      u8g.drawStr(0, 12 + i * 15, lines[i]);
    }
  } // max display
}

void Selection(int state, char choices[2][15]){  
  if (state == 1) {
    u8g.setFont(u8g_font_7x14B);   
    u8g.drawStr(34, 24, choices[0]); // Good Bold
    
    u8g.setFont(u8g_font_7x14);   
    u8g.drawStr(34, 45, choices[1]); // Bad
  } 
  else if (state == 2) {
    u8g.setFont(u8g_font_7x14);   
    u8g.drawStr(34, 24, choices[0]); // Good
    
    u8g.setFont(u8g_font_7x14B);   
    u8g.drawStr(34, 45, choices[1]); // Bad Bold
  }
}

void Waiting(){
  u8g.setFont(u8g_font_fur17);    
  u8g.drawStr(0, 27, "Waiting...");
}

void Stop(){
  u8g.setFont(u8g_font_fur17);    
  u8g.drawStr(0, 27, "Quitting...");
}

void loop() {

  bool pressState = digitalRead(ButtonPress);
  bool leftState = digitalRead(ButtonLeft);
  bool rightState = digitalRead(ButtonRight);

  if (leftState == LOW && !leftWasPressed) {
    state = 1; // Good
    leftWasPressed = true;
  }
  if (leftState == HIGH && leftWasPressed) {
    leftWasPressed = false;
  }

  if (rightState == LOW && !rightWasPressed) {
    state = 2; // Bad
    rightWasPressed = true;
  }
  if (rightState == HIGH && rightWasPressed) {
    rightWasPressed = false;
  }

  if (Serial.available()>0) {
    inputText = Serial.readString();

    if (inputText == "stop"){
      screen = 2;
    };
    
    question = "";
    String words[20];  
    int wordCount = 0;
    int startIndex = 0;
    for (int i = 0; i <= inputText.length(); i++) {
      if (inputText.charAt(i) == ' ' || i == inputText.length()) {
        words[wordCount++] = inputText.substring(startIndex, i);
        startIndex = i + 1;
      }
    }// Separating questions and choices

    if (wordCount >= 2) {
      choices[0] = words[wordCount - 2];
      choices[1] = words[wordCount - 1];

      for (int i = 0; i < wordCount - 2; i++) {
        question += words[i];
        if (i < wordCount - 3) question += " ";
      }
      
    } 
    choices[0].toCharArray(choicesChar[0], sizeof(choicesChar[0]));
    choices[1].toCharArray(choicesChar[1], sizeof(choicesChar[1]));
    question.toCharArray(questionChar, sizeof(questionChar)); // Converting to Char so that the functions accept and no err
  }

  u8g.firstPage();
  do {
    if (screen == 0){
      if (question.length()==0){
        Waiting();
      }else{
      Question(questionChar);
      if (pressState == LOW && !buttonWasPressed) {
        buttonWasPressed = true;
      }if (pressState == HIGH && buttonWasPressed) {
        screen = 1;
        buttonWasPressed = false;
      }}
    }else if (screen == 1){
      if (choices[0].length() == 0 || choices[1].length() ==0){
        Waiting();
      }else{
        Selection(state, choicesChar);

      if ((state != 0) && pressState == LOW && !buttonWasPressed) {
        buttonWasPressed = true;
      } 

      if ((state != 0) && pressState == HIGH && buttonWasPressed) {
        Serial.println(choices[state-1]);
        screen = 0;
        buttonWasPressed = false;
      }
      }
    }else if (screen == 2){
      Stop();
    }

  } while (u8g.nextPage());
}
