#include <AFMotor.h>
#include <Servo.h>

// ================= PIN =================
AF_DCMotor motorR(3);              // M3 = kanan
AF_DCMotor motorL(4);              // M4 = kiri
Servo gripper;
const uint8_t SERVO_PIN = 10;      
const uint8_t SENSOR_PIN[5] = {A0, A1, A2, A3, A4};  
const uint8_t BOX_PIN = A5;

// ======== ARAH MOTOR ========
#define DIR_FWD FORWARD
#define DIR_BWD BACKWARD

// ======== INVERSI ARAH MOTOR ========
const int INVERT_L = -1;           // M4 berputar mundur saat diperintah maju
const int INVERT_R = 1;            // Tetap 1 asumsi M3 sudah benar

// ================= SENSOR DIGITAL (TCRT5000) =================
const uint8_t LINE_LEVEL = HIGH;
const int8_t WEIGHT[5]   = {-2, -1, 0, 1, 2};   

const bool BOX_ACTIVE_LOW = true;  
const uint8_t BOX_DEBOUNCE = 3;

#define DEBUG_SENSOR false         

// ================= GRIPPER =================
const int GRIP_OPEN  = 0;
const int GRIP_CLOSE = 110;        
const int GRIP_STEP_DELAY = 15;    

// ================= PID & KECEPATAN =================
const float Kp = 45.0;
const float Kd = 90.0;
const int   BASE_SPEED_FREE  = 140;
const int   BASE_SPEED_CARRY = 120; 
const int   MAX_CORRECTION   = 150;
const unsigned long LOOP_MS  = 10;

// ================= MEMORI TIKUNGAN =================
const int  SEARCH_SPEED  = 195;   // Tenaga memutar badan saat mencari garis (pivot)

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
  setMotor(-90, -90);              
  delay(300);
  stopMotor();
  while (true) { }                 
}

// ---------- FUNGSI KUNCI MEMORI KESALAHAN (DIPERBAIKI) ----------
// dir: -1 = putar kiri, +1 = putar kanan
void memoryTurn(int8_t dir) {
  // Tetapkan kecepatan pencarian yang stabil (misalnya 160 atau 170 agar tidak terlalu liar)
  const int CONTROLLED_SPEED = 160;
  
  // Putar di tempat dengan dua roda berlawanan arah
  setMotor(dir * CONTROLLED_SPEED, -dir * CONTROLLED_SPEED);
  
  // FASE 1: PELEPASAN TERKONTROL
  // Beri waktu minimum wajib berputar (misalnya 80 milidetik) agar robot benar-benar lepas dari garis awal 
  // tanpa sempat berputar terlalu jauh. Sesuaikan angka milidetik ini di lapangan.
  delay(80); 
  
  // Lanjutkan menunggu sampai sensor tengah (S3) benar-benar KELUAR dari garis awal
  unsigned long tRelease = millis();
  while (millis() - tRelease < 300) { 
    readSensors();
    if (!s[2]) break; 
  }
  
  // FASE 2: PENCARIAN GARIS BARU
  unsigned long tFind = millis();
  while (millis() - tFind < 1500) { 
    readSensors();
    if (s[2]) break; // Garis baru ditemukan
  }
  
  stopMotor(); // Rem sebentar untuk menghentikan kelembaman (momentum) putaran
  delay(20);
  
  lastError = 0; 
}

void setup() {
  Serial.begin(9600);
  for (uint8_t i = 0; i < 5; i++) pinMode(SENSOR_PIN[i], INPUT);
  pinMode(BOX_PIN, INPUT);
  gripper.attach(SERVO_PIN);
  gripper.write(GRIP_OPEN);
  stopMotor();
  delay(1500);                     
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

  // --- deteksi box ---
  if (!holdingBox) {
    if (boxDetectedRaw()) boxCount++; else boxCount = 0;
    if (boxCount >= BOX_DEBOUNCE) {
      pickBox();
      return;
    }
  }

  // --- garis lintang penuh (finish/drop) ---
  if (count >= 4) { 
    if (holdingBox) {
      releaseBoxAndStop();
      return;
    }
    setMotor(BASE_SPEED_FREE, BASE_SPEED_FREE);
    return;
  }

  // --- TIKUNGAN TAJAM (MEMORI POLA S1,S2 / S4,S5) ---
  // Syarat dibuat sedikit longgar (S1 dan S2 saja cukup) agar robot yang melaju kencang tidak melewatkan titik pemicu
  if (s[0] && s[1] && !s[4]) {
     memoryTurn(-1); // Eksekusi memori belok kiri
     return;
  }
  if (s[4] && s[3] && !s[0]) {
     memoryTurn(1);  // Eksekusi memori belok kanan
     return;
  }

  // --- GARIS HILANG SEPENUHNYA (MEMORI KESALAHAN PID) ---
  if (count == 0) {
    if (lastError > 0) {
      memoryTurn(1);  // Pivot kanan sampai lurus
    } else {
      memoryTurn(-1); // Pivot kiri sampai lurus
    }
    return;
  }

  // --- PID NORMAL ---
  int sum = 0;
  for (uint8_t i = 0; i < 5; i++) if (s[i]) sum += WEIGHT[i];
  float error = (float)sum / count;                

  float correction = Kp * error + Kd * (error - lastError);
  correction = constrain(correction, -MAX_CORRECTION, MAX_CORRECTION);
  lastError = error;

  int base  = holdingBox ? BASE_SPEED_CARRY : BASE_SPEED_FREE;
  int left  = constrain(base + (int)correction, 0, 255);   
  int right = constrain(base - (int)correction, 0, 255);
  setMotor(left, right);
}