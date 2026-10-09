#include <AFMotor.h>

// ================= PIN MOTOR =================
AF_DCMotor motorR(3);              // M3 = Kanan
AF_DCMotor motorL(4);              // M4 = Kiri

// ======== ARAH MOTOR ========
#define DIR_FWD FORWARD
#define DIR_BWD BACKWARD

// Variabel pengali untuk membalik arah putaran jika terjadi ketidaksesuaian perangkat keras
const int INVERT_L = -1;           // Diubah menjadi -1 karena M4 berputar mundur saat diperintah maju
const int INVERT_R = 1;            // Tetap 1 asumsi M3 sudah benar (ubah ke -1 jika ikut terbalik)

const int TEST_SPEED = 100;
const unsigned long TEST_MS = 1500;

void driveMotor(AF_DCMotor &m, int spd) {
  spd = constrain(spd, -255, 255);
  if (spd > 0)      { m.setSpeed(spd);  m.run(DIR_FWD); }
  else if (spd < 0) { m.setSpeed(-spd); m.run(DIR_BWD); }
  else              { m.setSpeed(0);    m.run(RELEASE); }
}

void setMotor(int left, int right) {
  // Terapkan variabel pengali arah pada masing-masing motor
  driveMotor(motorL, left * INVERT_L);
  driveMotor(motorR, right * INVERT_R);
}

void stopMotor() { 
  setMotor(0, 0); 
}

void runTimed(int left, int right, unsigned long ms) {
  setMotor(left, right);
  delay(ms);
  stopMotor();
  Serial.println(F("Selesai. Cocokkan dengan hasil yang diharapkan."));
}

void printMenu() {
  Serial.println(F("\n===== MODE TES MOTOR ====="));
  Serial.println(F("1 : Motor KIRI (M4) maju   -> robot melengkung ke KANAN"));
  Serial.println(F("2 : Motor KANAN (M3) maju  -> robot melengkung ke KIRI"));
  Serial.println(F("3 : Kedua motor maju       -> robot MAJU LURUS"));
  Serial.println(F("4 : Kedua motor mundur     -> robot MUNDUR LURUS"));
  Serial.println(F("5 : Pivot kiri             -> berputar di tempat ke KIRI"));
  Serial.println(F("6 : Pivot kanan            -> berputar di tempat ke KANAN"));
}

void flushSerial() {
  delay(20);
  while (Serial.available()) Serial.read();
}

void setup() {
  Serial.begin(9600);
  stopMotor();
  printMenu();
}

void loop() {
  if (!Serial.available()) return;
  char c = Serial.read();
  if (c == '\n' || c == '\r' || c == ' ') return;
  flushSerial();

  switch (c) {
    case '1':
      Serial.println(F("Tes 1: hanya motor KIRI maju. Harapan: melengkung ke KANAN."));
      runTimed(TEST_SPEED, 0, TEST_MS);
      break;
    case '2':
      Serial.println(F("Tes 2: hanya motor KANAN maju. Harapan: melengkung ke KIRI."));
      runTimed(0, TEST_SPEED, TEST_MS);
      break;
    case '3':
      Serial.println(F("Tes 3: kedua motor maju. Harapan: MAJU LURUS."));
      runTimed(TEST_SPEED, TEST_SPEED, TEST_MS);
      break;
    case '4':
      Serial.println(F("Tes 4: kedua motor mundur. Harapan: MUNDUR LURUS."));
      runTimed(-TEST_SPEED, -TEST_SPEED, TEST_MS);
      break;
    case '5':
      Serial.println(F("Tes 5: pivot kiri. Harapan: berputar di tempat ke KIRI."));
      runTimed(-TEST_SPEED, TEST_SPEED, TEST_MS);
      break;
    case '6':
      Serial.println(F("Tes 6: pivot kanan. Harapan: berputar di tempat ke KANAN."));
      runTimed(TEST_SPEED, -TEST_SPEED, TEST_MS);
      break;
    default:
      Serial.println(F("Perintah tidak dikenal."));
      printMenu();
      break;
  }
}