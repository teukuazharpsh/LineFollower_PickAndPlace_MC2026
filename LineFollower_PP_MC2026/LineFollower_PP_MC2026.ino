#include <AFMotor.h>
#include <Servo.h>

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

// ======== INVERSI ARAH MOTOR ========
const int INVERT_L = -1;           // Diubah menjadi -1 karena M4 berputar mundur saat diperintah maju
const int INVERT_R = 1;            // Tetap 1 asumsi M3 sudah benar (ubah ke -1 jika ikut terbalik)

// ================= SENSOR DIGITAL (TCRT5000) =================
// Level output modul saat sensor berada di atas GARIS.
// Garis hitam di lantai putih: kebanyakan modul LM393 -> HIGH di hitam (LED indikator mati).
// Jika robot bereaksi terbalik, ubah ke LOW.
const uint8_t LINE_LEVEL = HIGH;
const int8_t WEIGHT[5]   = {-2, -1, 0, 1, 2};   // satuan pitch (20 mm)

const bool BOX_ACTIVE_LOW = true;  // modul IR umumnya LOW saat ada objek
const uint8_t BOX_DEBOUNCE = 3;

#define DEBUG_SENSOR false         // true: cetak pola sensor ke Serial Monitor (uji saja, motor tetap jalan)

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
const unsigned long FORWARD_MS    = 290;  // = 58,08 mm / kecepatan(mm/s), KALIBRASI!
const unsigned long MIN_PIVOT_MS  = 120;
const unsigned long PIVOT_TIMEOUT = 1500;
const int  SEARCH_SPEED  = 90;

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
  // Terapkan variabel pengali arah pada masing-masing motor
  driveMotor(motorL, left * INVERT_L);
  driveMotor(motorR, right * INVERT_R);
}

void stopMotor() { 
  setMotor(0, 0); 
}

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

void setup() {
  Serial.begin(9600);
  for (uint8_t i = 0; i < 5; i++) pinMode(SENSOR_PIN[i], INPUT);
  pinMode(BOX_PIN, INPUT);
  gripper.attach(SERVO_PIN);
  gripper.write(GRIP_OPEN);
  stopMotor();
  delay(1500);                     // waktu meletakkan robot di jalur
}

void loop() {
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