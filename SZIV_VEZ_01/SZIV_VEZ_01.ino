#include <SPI.h>
byte ledz;
byte k_ido;
byte cnt;
byte i;
int poti;
bool level1, level2, level3;
bool mot1, mot2, valt, cs1, cs2;
bool runled, automata, manual, autoled;
unsigned long previousMillis = 0;

void setup() {
  pinMode(A1, INPUT); //level1
  pinMode(A2, INPUT); //level2
  pinMode(A3, INPUT); //level3
  pinMode(9, OUTPUT); //motor1
  pinMode(8, OUTPUT); //motor2
  pinMode(7, INPUT);  //automata
  pinMode(6, INPUT);  //manual
  pinMode(10, OUTPUT); //SPI_SS
  SPI.begin();
  //Serial.begin(250000);

  while (1) {
    bitClear (PORTB, 2);
    SPI.transfer(analogRead(A6)/4);
    bitSet (PORTB, 2);
  }
}

void loop() {

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= 100) {
    i++;
    if (i > 10) {
      cs2 = 1;
      i = 0;
    }
    if (cs2) {
      runled = 1;
      autoled = 0;
      cs2 = 0;
      if (level1 == 0) {
        if (cnt < 200) cnt++;
      }
      else {
        cnt = 0;
      }
      //Serial.println(cnt);
    }
    else {
      runled = 0;
      if (automata) autoled = 1;
    }
    ledf();
    previousMillis = currentMillis;
  }

  level1 = digitalRead(A1);
  level2 = digitalRead(A2);
  level3 = digitalRead(A3);
  automata = digitalRead(7);
  manual = digitalRead(6);


  if (automata || manual) {

    poti = analogRead(A6) / 128;

    if (manual == 1) {
      mot1 = 1; mot2 = 1;
    }

    if (level1 == 0) {
      if (cnt > poti) {
        mot1 = 0; mot2 = 0;
      }
    }
    if (level2 == 1 && cs1 == 0) {
      if (valt == 0) {
        mot1 = 1;
      }
      else {
        mot2 = 1;
      }
      valt = !valt;
      cs1 = 1;
      mot();
      ledf();
      delay(250);
    }
    if (level2 == 0) {
      cs1 = 0;
    }
    if (level3 == 1) {
      mot1 = 1; mot2 = 1;
      delay(250);
    }
    mot();
  }
  else {
    mot1 = 0; mot2 = 0;
    mot();
  }
}

void mot() {
  digitalWrite(9, mot1);
  digitalWrite(8, mot2);
}

void ledf() {
  ledz = (runled << 7) | (autoled << 6) | (manual << 5) | (mot1 << 4) | (mot2 << 3) | (level1 << 2) | (level2 << 1) | level3;
  bitClear (PORTB, 2);
  SPI.transfer(ledz);
  bitSet (PORTB, 2);
}
