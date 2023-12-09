#include <SPI.h>
#include <SoftwareSerial.h>
SoftwareSerial gsm = SoftwareSerial(4, 5);

String telszam = "+36709421310";
char ok[] = "OK";
bool tovabb;
bool bet_hiba_timer_start, bet_hiba_timer_finish;
bool m1_hiba_timer_start, m1_hiba_timer_finish;
bool m2_hiba_timer_start, m2_hiba_timer_finish;
bool vmax_hiba_timer_start, vmax_hiba_timer_finish;
bool sms_m1h, sms_m2h, sms_m1h_flag, sms_m2h_flag, sms_betap, sms_vmax;
bool mot1_hiba, mot2_hiba, valt, mot_valtak, mot_valt_cs1, szint_kapcs;
bool mot1_out, mot2_out, SSR_out, aux_in, vmax_in, betap_in;
bool fut_LED, am_LED, hiba_LED, level1_LED, level2_LED, level3_LED, mot1_LED, mot2_LED;
byte loopcnt, LED_OUT, timer2, bet_hiba_timer, m1_hiba_timer, m2_hiba_timer, vmax_hiba_timer;
int auto_manual_in, motorhiba_in, szint_in, utanfutas_in;
unsigned long timer1 = 0;

void setup() {
  sms_betap = true;
  pinMode(A0, INPUT);  // Betáp
  pinMode(A1, INPUT);  // Automata-kézi üzem
  pinMode(A2, INPUT);  // Motorhibák
  pinMode(A3, INPUT);  // 4-20mA szint
  pinMode(A6, INPUT);  // utánfutás poti
  pinMode(6,  INPUT);  // Aux
  pinMode(7,  INPUT);  // Vmax
  pinMode(3,  OUTPUT); // SS RELÉ - hiba kimenet
  pinMode(8,  OUTPUT); // motor1 - kimenet
  pinMode(9,  OUTPUT); // motor2 - kimenet
  pinMode(10, OUTPUT); // SPI_SS
  SPI.begin();
  Serial.begin(57600);
  gsm.begin(57600);

  while (1) {

    if (valt) {
      for (byte i = 0; i < 10; i++) analogReference(INTERNAL);
      for (byte i = 0; i < 3; i++) auto_manual_in =  analogRead(A1);
      for (byte i = 0; i < 3; i++) motorhiba_in   =  analogRead(A2);
      for (byte i = 0; i < 3; i++) szint_in       =  analogRead(A3);
    }
    else {
      for (byte i = 0; i < 10; i++) analogReference(DEFAULT);
      for (byte i = 0; i < 3; i++) utanfutas_in   =  analogRead(A6);
    }

    betap_in       =  digitalRead(A0);
    aux_in         =  digitalRead(6);
    vmax_in        =  digitalRead(7);

    if (millis() - timer1 > 20) {

      // HeartBeat kijezlése a FUT_LED-re ha van betáp, 2 másodpercenként felvillantva:
      if ((loopcnt == 0 || loopcnt == 15) && betap_in == 1) {
        fut_LED = true;
      }
      else {
        fut_LED = false;
      }

      // Szint mérés kijelzése
      if (szint_in > 650) {
        level3_LED = true;
        level2_LED = true;
        level1_LED = true;
        mot1_out = true;
        mot2_out = true;
        mot_valt_cs1 = true;
        timer2 = 0;
      }
      else if (szint_in > 450) {
        level3_LED = false;
        level2_LED = true;
        level1_LED = true;
        if (mot_valt_cs1) {
          mot_valtak = !mot_valtak;
          mot_valt_cs1 = false;
        }
        if (mot_valtak) {
          mot1_out = true;
          if (timer2 >= (utanfutas_in / 32) + 1) {
            mot2_out = true;
          }
          else {
            mot2_out = false;
          }
        }
        else {
          if (timer2 >= (utanfutas_in / 32) + 1) {
            mot1_out = true;
          }
          else {
            mot1_out = false;
          }
          mot2_out = true;
        }
        szint_kapcs = true;
      }
      else if (szint_in > 250) {
        level3_LED = false;
        level2_LED = false;
        level1_LED = true;
        timer2 = 0;
        // motor előző állapotát tartja...
        if (szint_kapcs) {
          if (mot_valtak) {
            mot1_out = true;
            mot2_out = false;
          }
          else {
            mot1_out = false;
            mot2_out = true;
          }
        }
      }
      else {
        level3_LED = false;
        level2_LED = false;
        level1_LED = false;
        mot1_out = false;
        mot2_out = false;
        szint_kapcs = false;
        mot_valt_cs1 = true;
        timer2 = 0;
      }

      // Auto - Manual mod valaszto:
      if (auto_manual_in > 700) {
        mot1_out = true;
        mot2_out = true;
        am_LED = true;
      }
      else if (auto_manual_in > 450) {
        mot1_out = false;
        mot2_out = true;
        am_LED = true;
      }
      else if (auto_manual_in > 300) {
        mot1_out = true;
        mot2_out = false;
        am_LED = true;
      }
      else if (auto_manual_in > 100) {
        am_LED = false;
      }
      else {
        if (betap_in) {
          hiba_LED = true;
          am_LED = true;
        }
        else {
          am_LED = false;
        }
      }

      // Motor hiba valasztas:
      if (motorhiba_in > 700) {
        sms_m2h = true;
        sms_m1h = true;
        mot2_hiba = true;
        mot1_hiba = true;
      }
      else if (motorhiba_in > 450) {
        sms_m2h = true;
        sms_m1h = false;
        mot2_hiba = true;
        mot1_hiba = false;
      }
      else if (motorhiba_in > 300) {
        sms_m2h = false;
        sms_m1h = true;
        mot2_hiba = false;
        mot1_hiba = true;
      }
      else if (motorhiba_in > 100) {
        if (betap_in) {
          sms_m2h = false;
          sms_m1h = false;
          mot2_hiba = false;
          mot1_hiba = false;
        }
      }
      else {
        if (betap_in) {
          mot2_hiba = true;
          mot1_hiba = true;
        }
        else {
          mot2_hiba = false;
          mot1_hiba = false;
        }
      }

      // picit több késleltetés, 500ms -onként teljesül :
      if (loopcnt == 0 || loopcnt == 25 || loopcnt == 50 || loopcnt == 75) {
        // AD konverzió referencia kapcsolgatása
        valt = !valt;

        // Ha nincs betáp akkor villogó hibajelzés:
        if (betap_in == 0) {
          hiba_LED = true;
        }


        // Vész Maximum esetén Motorokat bekapcsolja, Level3 LED villog:
        if (vmax_in) {
          mot1_out = true;
          mot2_out = true;
          level3_LED = true;
        }

        // Motorhiba esetén a hozzátartozó LED villogtatása:
        if (mot1_hiba) {
          mot1_LED = true;
          hiba_LED = true;
          mot1_out = false;
        }
        else {
          mot1_LED = mot1_out;
        }
        if (mot2_hiba) {
          mot2_LED = true;
          hiba_LED = true;
          mot2_out = false;
        }
        else {
          mot2_LED = mot2_out;
        }

        digitalWrite(8, mot1_out);  // motor 1 vezérlése
        digitalWrite(9, mot2_out);  // motor 2 vezérlése
        /*
          Serial.print("Mot1: ");
          Serial.print(mot1_out);
          Serial.print("\t");
          Serial.print("Mot2: ");
          Serial.print(mot2_out);
          Serial.println("\t");
        */
        mot1_out = false;
        mot2_out = false;
      }
      else {
        if (mot1_hiba) {
          mot1_LED = false;
        }
        if (mot2_hiba) {
          mot2_LED = false;
        }
        if (mot1_hiba == 0 && mot2_hiba == 0) {
          hiba_LED = false;
        }
        if (szint_in > 650 && vmax_in) {
          level3_LED = false;
        }
        if (auto_manual_in < 100) {
          am_LED = false;
        }
      }

      digitalWrite(3, hiba_LED);  // SSR relé vezérlése
      LED_OUT = (fut_LED << 7) | (am_LED << 6) | (hiba_LED << 5) | (mot1_LED << 4) | (mot2_LED << 3) | (level1_LED << 2) | (level2_LED << 1) | level3_LED;
      bitClear (PORTB, 2);
      SPI.transfer(LED_OUT);
      bitSet (PORTB, 2);
      /*
            Serial.print("pot: ");
            Serial.print(utanfutas_in / 32);
            Serial.print("\t");

            Serial.print("A/M: ");
            Serial.print(auto_manual_in);
            Serial.print("\t");

            Serial.print("M_hiba: ");
            Serial.print(motorhiba_in);
            Serial.print("\t");

            Serial.print("Szint: ");
            //Serial.print(float(szint_in * 19.64 / 1024));
            Serial.print(szint_in);
            Serial.print("\t");

            Serial.print((betap_in << 7) | (aux_in << 6) | (vmax_in << 5) | (mot1_LED << 4) | (mot2_LED << 3) | (level1_LED << 2) | (level2_LED << 1) | level3_LED, BIN);

            Serial.println();
      */


      if (loopcnt > 99) {

        // váltakozva indulás időzítője:
        if (timer2 < 255) timer2++;

        // betap hiba timer
        if (bet_hiba_timer_start) {
          if (bet_hiba_timer < 3) {
            bet_hiba_timer++;
          }
          else {
            bet_hiba_timer_finish = true;
            bet_hiba_timer_start = false;
            bet_hiba_timer = 0;
          }
        }
        else {
          bet_hiba_timer = 0;
        }

        // motor 1 hiba timer
        if (m1_hiba_timer_start) {
          if (m1_hiba_timer < 3) {
            m1_hiba_timer++;
          }
          else {
            m1_hiba_timer_finish = true;
            m1_hiba_timer_start = false;
            m1_hiba_timer = 0;
          }
        }
        else {
          m1_hiba_timer = 0;
        }

        // motor 2 hiba timer
        if (m2_hiba_timer_start) {
          if (m2_hiba_timer < 3) {
            m2_hiba_timer++;
          }
          else {
            m2_hiba_timer_finish = true;
            m2_hiba_timer_start = false;
            m2_hiba_timer = 0;
          }
        }
        else {
          m2_hiba_timer = 0;
        }

        // vmax hiba timer
        if (vmax_hiba_timer_start) {
          if (vmax_hiba_timer < 3) {
            vmax_hiba_timer++;
          }
          else {
            vmax_hiba_timer_finish = true;
            vmax_hiba_timer_start = false;
            vmax_hiba_timer = 0;
          }
        }
        else {
          vmax_hiba_timer = 0;
        }

        loopcnt = 0;
      }
      else {
        loopcnt++;
      }
      timer1 = millis();
    }

    //SMS küldés:

    // betáp hiba esetén:
    if (betap_in != sms_betap) {
      bet_hiba_timer_start = true;
      if (bet_hiba_timer_finish) {
        if (!betap_in) {
          Serial.println(F("ARAM HIBA!"));
        }
        else {
          Serial.println(F("ARAM OK..."));
        }
        sms_betap = betap_in;
        bet_hiba_timer_finish = false;
      }
    }
    else {
      bet_hiba_timer_start = false;
    }

    // 1. Motorhiba esetén
    if (sms_m1h != sms_m1h_flag) {
      m1_hiba_timer_start = true;
      if (m1_hiba_timer_finish) {
        if (sms_m1h) {
          Serial.println(F(" 1. MOTOR HIBA"));
        }
        else {
          Serial.println(F(" 1. MOTOR OK..."));
        }
        sms_m1h_flag = sms_m1h;
        m1_hiba_timer_finish = false;
      }
    }
    else {
      m1_hiba_timer_start = false;
    }

    // 2. Motorhiba esetén
    if (sms_m2h != sms_m2h_flag) {
      m2_hiba_timer_start = true;
      if (m2_hiba_timer_finish) {
        if (sms_m2h) {
          Serial.println(F(" 2. MOTOR HIBA"));
        }
        else {
          Serial.println(F(" 2. MOTOR OK..."));
        }
        sms_m2h_flag = sms_m2h;
        m2_hiba_timer_finish = false;
      }
    }
    else {
      m2_hiba_timer_start = false;
    }


    // vmax hiba esetén:
    if (vmax_in != sms_vmax) {
      vmax_hiba_timer_start = true;
      if (vmax_hiba_timer_finish) {
        if (vmax_in) {
          Serial.println(F("KRITIKUS SZINT!"));
          /*
            gsm.flush();
            Serial.print(F("GSM modul kereses...\t"));
            tovabb = false;
            while (!tovabb) {
            if (gsm.find(ok)) {
              Serial.println(ok);
              tovabb = true;
            }
            else {
              gsm.println(F("AT"));
              delay(100);
            }
            }
          */
          gsm.println("AT"); // HandShake
          delay(500);
          updateSerial();
          gsm.println("AT+CMGF=1"); // TEXT mode
          delay(500);
          updateSerial();
          gsm.println("AT+CMGS=\"" + telszam + "\"");
          gsm.print("KRITIKUS SZINT!"); //text content
          delay(500);
          updateSerial();
          gsm.write(26);
        }
        else {
          Serial.println(F("SZINT OK..."));
        }
        sms_vmax = vmax_in;
        vmax_hiba_timer_finish = false;
      }
    }
    else {
      vmax_hiba_timer_start = false;
    }

    updateSerial();

  }

}

void loop() {
}

void updateSerial()
{
  while (Serial.available())
  {
    gsm.write(Serial.read());//Forward what Serial received to Software Serial Port
  }
  while (gsm.available())
  {
    Serial.write(gsm.read());//Forward what Software Serial received to Serial Port
  }
}
