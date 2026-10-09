#include <AFMotor.h>
#include <Servo.h>

// ================= MODE =================
#define MODE_TEST 0
#define MODE_RUN  1
const uint8_t START_MODE = MODE_TEST;   // MODE_TEST = aman untuk uji; ketik 'r' untuk menjalankan
uint8_t currentMode = START_MODE;

// ================= PIN =================
AF_DCMotor motorR(3);              // M3 = kanan
AF_DCMotor motorL(4);              // M4 = kiri
Servo gripper;
const uint8_t SERVO_PIN = 10;      // header SERVO_1 pada shield
const uint8_t SENSOR_PIN[5] = {A0, A1, A2, A3, A4};  // S1(kiri) ... S5(kanan)
const uint8_t BOX_PIN = A5;

// ======== ARAH MOTOR (tukar jika arah terbalik) ========
#define DIR_FWD FORWARD
#define DIR_BWD BACKWARD

// ================= SENSOR DIGITAL (TCRT5000) =================
// Level output modul saat sensor berada di atas GARIS.
// Garis hitam di lantai putih: kebanyakan modul LM393 -> HIGH di hitam (LED indikator mati).
// Jika robot bereaksi terbalik, ubah ke LOW.
const uint8_t LINE_LEVEL = HIGH;
const int8_t WEIGHT[5]   = {-2, -1, 0, 1, 2};   // satuan pitch (20 mm)

const bool BOX_ACTIVE_LOW = true;  // modul IR umumnya LOW saat ada objek
const uint8_t BOX_DEBOUNCE = 3;

#define DEBUG_SENSOR false         // true: cetak pola sensor saat MODE_RUN (motor tetap jalan)

// ================= GRIPPER =================
const int GRIP_OPEN  = 0;
const int GRIP_CLOSE = 110;        // harus 90 s.d. 120
const int GRIP_STEP_DELAY = 15;    // ms per 2 derajat

// ================= PID =================
const float Kp = 25.0;
const float Kd = 60.0;
const int   BASE_SPEED_FREE  = 110;
const int   BASE_SPEED_CARRY = 90; // lebih pelan saat membawa box
const int   MAX_CORRECTION   = 90;
const unsigned long LOOP_MS  = 10;

// ================= TIKUNGAN =================
const int  TURN_SPEED    = 110;
const int  ADVANCE_SPEED = 100;
const unsigned long FORWARD_MS    = 290;  // = 58,08 mm / kecepatan(mm/s), KALIBRASI! (perintah 'k')
const unsigned long MIN_PIVOT_MS  = 120;
const unsigned long PIVOT_TIMEOUT = 1500;
const int  SEARCH_SPEED  = 90;

// ================= PARAMETER MODE TES =================
const int  TEST_SPEED   = 100;            // PWM untuk tes motor
const unsigned long TEST_MS = 1500;       // lama tiap tes motor (ms)
const unsigned long KAL_MS  = 2000;       // lama maju saat kalibrasi (ms)
const float L_SENSOR_MM = 58.08;          // jarak as roda ke sensor (mm)

// ================= STATUS =================
float lastError = 0;
bool  holdingBox = false;
uint8_t boxCount = 0;
unsigned long lastLoop = 0;
bool s[5];

// ---------- motor ----------
void driveMotor(AF_DCMotor &m, int spd) {
  spd = constrain(spd, -255, 255);
  if (spd > 0)      { m.setSpeed(spd);  m.run(DIR_FWD); }
  else if (spd < 0) { m.setSpeed(-spd); m.run(DIR_BWD); }
  else              { m.setSpeed(0);    m.run(RELEASE); }
}

void setMotor(int left, int right) {
  driveMotor(motorL, left);
  driveMotor(motorR, right);
}

void stopMotor() { setMotor(0, 0); }

// ---------- sensor ----------
uint8_t readSensors() {
  uint8_t count = 0;
  for (uint8_t i = 0; i < 5; i++) {
    s[i] = (digitalRead(SENSOR_PIN[i]) == LINE_LEVEL);
    if (s[i]) count++;
  }
  return count;
}

bool boxDetectedRaw() {
  bool level = digitalRead(BOX_PIN);
  return BOX_ACTIVE_LOW ? (level == LOW) : (level == HIGH);
}

// ---------- gripper ----------
void moveGripper(int from, int to) {
  int step = (to > from) ? 2 : -2;
  for (int a = from; (step > 0) ? (a < to) : (a > to); a += step) {
    gripper.write(a);
    delay(GRIP_STEP_DELAY);
  }
  gripper.write(to);
  delay(200);
}

void pickBox() {
  stopMotor();
  delay(200);
  moveGripper(GRIP_OPEN, GRIP_CLOSE);
  holdingBox = true;
  lastError = 0;
}

void releaseBoxAndStop() {
  stopMotor();
  delay(200);
  moveGripper(GRIP_CLOSE, GRIP_OPEN);
  setMotor(-90, -90);              // mundur sebentar agar lepas dari box
  delay(300);
  stopMotor();
  while (true) { }                 // selesai
}

// ---------- tikungan tajam ----------
// dir: -1 = kiri, +1 = kanan
void handleCorner(int8_t dir) {
  stopMotor();
  delay(30);
  setMotor(ADVANCE_SPEED, ADVANCE_SPEED);   // as roda menuju titik sudut
  delay(FORWARD_MS);
  stopMotor();
  delay(30);

  readSensors();
  if (s[2]) { lastError = 0; return; }      // masih ada garis lurus (persimpangan)

  setMotor(dir * TURN_SPEED, -dir * TURN_SPEED);
  delay(MIN_PIVOT_MS);
  unsigned long t0 = millis();
  while (millis() - t0 < PIVOT_TIMEOUT) {
    readSensors();
    if (s[2]) break;                        // sensor tengah menemukan garis
  }
  stopMotor();
  delay(30);
  lastError = 0;
}

// =====================================================
//                    MODE TES
// =====================================================
void printMenu() {
  Serial.println(F("\n===== MODE TES ====="));
  Serial.println(F("1 : Motor KIRI (M4) maju   -> robot melengkung ke KANAN"));
  Serial.println(F("2 : Motor KANAN (M3) maju  -> robot melengkung ke KIRI"));
  Serial.println(F("3 : Kedua motor maju       -> robot MAJU LURUS"));
  Serial.println(F("4 : Kedua motor mundur     -> robot MUNDUR LURUS"));
  Serial.println(F("5 : Pivot kiri             -> berputar di tempat ke KIRI"));
  Serial.println(F("6 : Pivot kanan            -> berputar di tempat ke KANAN"));
  Serial.println(F("7 : Simulasi koreksi PID error positif -> melengkung ke KANAN"));
  Serial.println(F("s : Monitor sensor garis + box (ketik karakter apa pun untuk berhenti)"));
  Serial.println(F("g : Tes gripper (buka - tutup - buka)"));
  Serial.println(F("k : Kalibrasi FORWARD_MS"));
  Serial.println(F("r : JALANKAN program utama (hitung mundur 3 detik)"));
  Serial.println(F("h : Tampilkan menu"));
  Serial.println(F("Tes motor berhenti otomatis. Angkat/tahan robot jika perlu."));
}

void flushSerial() {
  delay(20);
  while (Serial.available()) Serial.read();
}

void runTimed(int left, int right, unsigned long ms) {
  setMotor(left, right);
  delay(ms);
  stopMotor();
  Serial.println(F("Selesai. Cocokkan dengan hasil yang diharapkan."));
}

void printSensors() {
  readSensors();
  for (uint8_t i = 0; i < 5; i++) Serial.print(s[i] ? '1' : '0');
  Serial.print(F("  box="));
  Serial.println(boxDetectedRaw() ? F("ADA") : F("tidak"));
}

void testSensorMonitor() {
  Serial.println(F("Monitor sensor (S1..S5, 1 = garis). Ketik karakter apa pun untuk berhenti."));
  flushSerial();
  while (!Serial.available()) {
    printSensors();
    delay(100);
  }
  flushSerial();
  Serial.println(F("Monitor dihentikan."));
}

void testGripper() {
  Serial.println(F("Gripper: buka -> tutup -> buka"));
  moveGripper(GRIP_CLOSE, GRIP_OPEN);   // pastikan posisi awal terbuka
  delay(500);
  moveGripper(GRIP_OPEN, GRIP_CLOSE);
  delay(1000);
  moveGripper(GRIP_CLOSE, GRIP_OPEN);
  Serial.println(F("Selesai."));
}

void testCalibrate() {
  Serial.print(F("Letakkan robot di lantai rata. Robot maju "));
  Serial.print(KAL_MS / 1000.0, 1);
  Serial.println(F(" detik dalam 3 detik..."));
  delay(3000);
  setMotor(ADVANCE_SPEED, ADVANCE_SPEED);
  delay(KAL_MS);
  stopMotor();
  Serial.println(F("Ukur jarak tempuh (mm) lalu ketik angkanya dan tekan Enter:"));
  flushSerial();
  Serial.setTimeout(60000);
  long mm = Serial.parseInt();
  Serial.setTimeout(1000);
  flushSerial();
  if (mm <= 0) {
    Serial.println(F("Input tidak valid atau waktu habis. Kalibrasi dibatalkan."));
    return;
  }
  float v  = (float)mm * 1000.0 / KAL_MS;          // mm/s
  float fm = L_SENSOR_MM / v * 1000.0;             // ms
  Serial.print(F("Kecepatan  : ")); Serial.print(v, 1);  Serial.println(F(" mm/s"));
  Serial.print(F("FORWARD_MS : ")); Serial.print(fm, 0); Serial.println(F(" ms"));
  Serial.println(F("Isikan nilai tersebut ke konstanta FORWARD_MS, lalu unggah ulang program."));
}

void startRun() {
  Serial.println(F("Menjalankan program utama. Letakkan robot di jalur..."));
  for (int i = 3; i > 0; i--) {
    Serial.println(i);
    delay(1000);
  }
  lastError = 0;
  holdingBox = false;
  boxCount = 0;
  gripper.write(GRIP_OPEN);
  currentMode = MODE_RUN;
  Serial.println(F("GO. Untuk kembali ke mode tes, tekan tombol reset."));
}

void testLoop() {
  if (!Serial.available()) return;
  char c = Serial.read();
  if (c == '\n' || c == '\r' || c == ' ') return;
  flushSerial();

  switch (c) {
    case '1':
      Serial.println(F("Tes 1: hanya motor KIRI maju. Harapan: robot melengkung ke KANAN."));
      runTimed(TEST_SPEED, 0, TEST_MS);
      break;
    case '2':
      Serial.println(F("Tes 2: hanya motor KANAN maju. Harapan: robot melengkung ke KIRI."));
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
    case '7':
      Serial.println(F("Tes 7: koreksi PID error positif (garis di kanan). Harapan: melengkung ke KANAN."));
      runTimed(BASE_SPEED_FREE + 40, BASE_SPEED_FREE - 40, TEST_MS);
      break;
    case 's': case 'S':
      testSensorMonitor();
      break;
    case 'g': case 'G':
      testGripper();
      break;
    case 'k': case 'K':
      testCalibrate();
      break;
    case 'r': case 'R':
      startRun();
      break;
    case 'h': case 'H': case '?':
      printMenu();
      break;
    default:
      Serial.println(F("Perintah tidak dikenal. Ketik h untuk menu."));
      break;
  }
}

// =====================================================
//                 PROGRAM UTAMA (MODE RUN)
// =====================================================
void runProgram() {
  if (millis() - lastLoop < LOOP_MS) return;
  lastLoop = millis();

  uint8_t count = readSensors();

#if DEBUG_SENSOR
  for (uint8_t i = 0; i < 5; i++) Serial.print(s[i] ? '1' : '0');
  Serial.print("  box=");
  Serial.println(boxDetectedRaw());
#endif

  // --- deteksi box (debounce) ---
  if (!holdingBox) {
    if (boxDetectedRaw()) boxCount++; else boxCount = 0;
    if (boxCount >= BOX_DEBOUNCE) {
      pickBox();
      return;
    }
  }

  // --- semua sensor kena garis ---
  if (count == 5) {
    if (holdingBox) releaseBoxAndStop();
    setMotor(ADVANCE_SPEED, ADVANCE_SPEED);
    return;
  }

  // --- tikungan 90 derajat: 11100 / 00111 ---
  if (s[0] && s[1] && !s[4]) { handleCorner(-1); return; }
  if (s[4] && s[3] && !s[0]) { handleCorner(+1); return; }

  // --- garis hilang: cari ke arah error terakhir ---
  if (count == 0) {
    if (lastError >= 0) setMotor(SEARCH_SPEED, -SEARCH_SPEED);   // putar kanan
    else                setMotor(-SEARCH_SPEED, SEARCH_SPEED);   // putar kiri
    return;
  }

  // --- PID ---
  int sum = 0;
  for (uint8_t i = 0; i < 5; i++) if (s[i]) sum += WEIGHT[i];
  float error = (float)sum / count;                // -2 s.d. +2 pitch

  float correction = Kp * error + Kd * (error - lastError);
  correction = constrain(correction, -MAX_CORRECTION, MAX_CORRECTION);
  lastError = error;

  int base  = holdingBox ? BASE_SPEED_CARRY : BASE_SPEED_FREE;
  int left  = constrain(base + (int)correction, 0, 255);   // error + = garis di kanan, belok kanan
  int right = constrain(base - (int)correction, 0, 255);
  setMotor(left, right);
}

// =====================================================
void setup() {
  Serial.begin(9600);
  for (uint8_t i = 0; i < 5; i++) pinMode(SENSOR_PIN[i], INPUT);
  pinMode(BOX_PIN, INPUT);
  gripper.attach(SERVO_PIN);
  gripper.write(GRIP_OPEN);
  stopMotor();

  if (currentMode == MODE_TEST) {
    printMenu();
  } else {
    delay(1500);                   // waktu meletakkan robot di jalur
  }
}

void loop() {
  if (currentMode == MODE_TEST) testLoop();
  else                          runProgram();
}