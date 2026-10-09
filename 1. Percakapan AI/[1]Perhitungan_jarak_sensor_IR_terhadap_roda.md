# Perhitungan jarak sensor IR terhadap roda

> **Provider:** Claude  
> **Date:** 10/9/2026, 9:30:10 PM  
> **Model:** claude-sonnet-5-5  
> **Source:** [Claude.ai](https://claude.ai/chat/8ce95823-e128-4091-9fa5-a0f33297f186)  
> **Active leaf:** `01a11c3e-b7b9-7af0-9a80-90009552eb80`  

---

## [1] USER

*10/8/2026, 10:18:51 PM*

`ID: 01a11c18-72d7-7467-841b-1f38af9cfa2e`

perhitungan jarak sebuah sensor ir terhadap roda 2 roda belakang  dan 1 caster wheel di depan bagaiaman cara mengetahui?

---

## [2] CLAUDE

*10/8/2026, 10:19:02 PM*

`ID: 01a11c18-72d7-7de1-b214-4773b833418b`

Pertanyaannya tentang jarak sensor IR terhadap roda belakang (2 roda penggerak) dan caster wheel di depan. Ini sebenarnya masalah **geometri chassis** dan pengaruhnya ke kontrol line follower. Berikut cara menentukannya.

## 1. Tentukan titik acuan (origin)

Ambil titik tengah **sumbu roda belakang** (garis yang menghubungkan pusat kedua roda penggerak) sebagai titik nol. Ini adalah titik robot berputar (pivot) saat kedua roda berlawanan arah.

- Sumbu X: ke depan robot
- Sumbu Y: ke kiri/kanan robot

## 2. Ukur langsung (cara paling praktis)

Dengan penggaris/jangka sorong, ukur:

| Besaran | Cara ukur |
|---|---|
| **L_sensor** | Jarak dari sumbu roda belakang ke garis sensor IR (tegak lurus sumbu roda) |
| **L_caster** | Jarak dari sumbu roda belakang ke titik kontak caster |
| **T (track width)** | Jarak antar pusat kedua roda belakang |
| **w_sensor** | Jarak antar sensor paling kiri dan paling kanan (lebar array) |
| **s** | Jarak antar sensor yang bersebelahan (pitch) |

Pastikan pengukuran ke **titik kontak dengan lantai** (bukan ke badan roda atau braket), dan ukur sejajar sumbu memanjang robot.

## 3. Jika dari desain CAD

Di Fusion 360/SolidWorks, buat sketsa tampak atas, lalu gunakan *Measure*:
- Pusat roda belakang → pusat sensor
- Pusat roda belakang → titik kontak caster

Kalau ada perubahan desain, angka ini ikut berubah, jadi catat di dokumentasi.

## 4. Hitung pengaruhnya ke gerak robot

**Sudut koreksi saat sensor mendeteksi error lateral e:**

$$\theta = \arctan\left(\frac{e}{L_{sensor}}\right)$$

**Radius belok minimum** (roda dalam diam, roda luar berputar):

$$R = \frac{T}{2}$$ (pivot di tengah), atau $$R = T$$ (satu roda diam).

**Panjang wheelbase efektif:**

$$L_{wb} = L_{caster}$$

## 5. Pertimbangan desain (rule of thumb)

- **Sensor lebih jauh dari sumbu roda** (L_sensor besar): sensor lebih cepat "melihat" belokan dan respons lebih awal, tetapi robot cenderung berosilasi (wobble) pada kecepatan tinggi. Butuh Kd lebih besar.
- **Sensor dekat sumbu roda**: lebih stabil, tapi telat bereaksi di tikungan tajam.
- Biasanya L_sensor sekitar **0,5 sampai 1,5 kali track width T** sebagai titik awal, lalu disetel lewat uji coba.
- **Caster di depan** membuat robot stabil dan sensor sebaiknya diletakkan **di antara caster dan roda** atau sedikit di depan caster, tergantung strategi. Pastikan sensor tidak terhalang caster dan tinggi sensor ke lantai sekitar **3 sampai 10 mm** untuk sensor IR pantul.
- Untuk lomba dengan tikungan tajam 90°, **pitch sensor s** sebaiknya lebih kecil dari lebar garis (misalnya garis 18 mm, pitch 10 sampai 15 mm) supaya tidak ada celah deteksi.

## 6. Verifikasi di lapangan

1. Letakkan robot di atas garis lurus, tandai posisi roda dan sensor dengan tape.
2. Putar robot di tempat, lihat apakah sensor menyapu lintasan sesuai perhitungan.
3. Sesuaikan jarak, lalu catat nilai terbaik untuk konfigurasi PID.

Kalau Anda sebutkan data robotnya (track width, jumlah sensor, kecepatan target), saya bisa bantu hitung L_sensor yang ideal dan estimasi nilai awal PID-nya.

---

## [3] USER

*10/8/2026, 10:43:18 PM*

`ID: 01a11c2e-bede-71ff-8532-956a63b28275`

jarak antara roda penggerak belakang ke casther wheel adalah 92.73 dan jarak antara roda penggerak ke sensor IR adalah 58.08 apaka aman jika ada tikung tajam maupun track lainnya?

---

## [4] CLAUDE

*10/8/2026, 10:43:40 PM*

`ID: 01a11c2e-bede-7dfb-9c49-976627dcdc45`

**Ya, secara geometri konfigurasinya aman**, tapi ada beberapa hal yang perlu diperhatikan untuk tikungan tajam.

## Penilaian konfigurasi

- **Sensor (58,08 mm) berada di antara roda dan caster (92,73 mm).** Ini bagus: caster tetap menjadi tumpuan ketiga yang stabil, dan sensor tidak terhalang. Selisih sensor ke caster adalah 92,73 − 58,08 = **34,65 mm**.
- **L_sensor tergolong pendek sampai sedang.** Robot lebih stabil dan tidak mudah berosilasi, tetapi "melihat" tikungan lebih lambat. Untuk track lomba biasa (tikungan 90°, sudut tajam, S-curve) ini aman, asalkan kecepatan dan PID disetel.
- Contoh sudut koreksi: jika error lateral 20 mm, θ = arctan(20/58,08) ≈ **19°**. Ini cukup responsif.

## Hal yang perlu diwaspadai

**1. Tikungan 90° dan sudut tajam.**
Saat sensor mendeteksi sudut, as roda masih sekitar **58 mm di belakang** sudut tersebut. Kalau langsung pivot, sensor menyapu busur dan bisa meleset dari garis baru. Solusinya:
- Setelah mendeteksi sudut, **maju sekitar 55 sampai 60 mm** (setara L_sensor) dulu, baru pivot (kedua roda berlawanan arah) sampai sensor tengah menemukan garis lagi.
- Atau pakai belok melengkung dengan roda dalam diperlambat, tapi ini butuh tikungan yang lebih lebar.

**2. Kecepatan.**
Waktu antisipasi sensor = L_sensor / v. Pada 0,5 m/s hanya sekitar 116 ms, dan pada 1 m/s sekitar 58 ms. Makin cepat, makin sedikit waktu bereaksi, jadi pelankan robot sebelum tikungan tajam (misalnya dengan memakai pola pembacaan sensor sebagai pemicu).

**3. Lebar array sensor.**
Cek apakah lebar array cukup untuk garis + persimpangan (T, +). Pitch antar sensor sebaiknya lebih kecil dari lebar garis. Untuk sudut tajam, sensor paling kiri dan kanan harus bisa menangkap garis belokan.

**4. Sapuan caster saat pivot.**
Saat berputar di tempat, caster menyapu lingkaran berjari-jari **92,73 mm** dari pusat as roda. Pastikan area itu bebas dari dinding, objek, atau garis tebal yang bisa menyangkut. Lengan/gripper di depan (yang terlihat menjulur ke kanan di gambar) menyapu lingkaran lebih besar lagi, jadi pastikan tidak menabrak objek saat belok.

**5. Titik berat (CoG).**
Karena ada lengan di depan caster, pastikan CoG robot **di antara as roda dan caster**. Kalau CoG terlalu maju (melewati caster), robot bisa terguling ke depan saat rem mendadak. Baterai dan komponen berat sebaiknya ditaruh agak ke belakang atau dekat as roda.

**6. Tinggi sensor.**
Dari gambar, sensor terpasang di bawah chassis dengan braket. Pastikan jarak ke lantai sekitar **3 sampai 10 mm** dan tidak berubah saat robot mengerem atau menikung (caster bisa membuat bodi sedikit miring).

## Langkah selanjutnya

Kalau Anda beri tahu **track width (jarak antar roda belakang)**, **jumlah sensor + pitch-nya**, dan **kecepatan target**, saya bisa hitung rasio L_sensor/T, menentukan jarak "maju sebelum pivot" yang tepat, dan memberi nilai awal Kp/Kd untuk PID-nya.

---

## [5] USER

*10/8/2026, 10:53:11 PM*

`ID: 01a11c37-e3e4-7792-b543-462478d7bab4`

track width: 3 cm, 
jarak antar roda belakang: 100 mm
jumlah sensor : 5
pitch / sensor: 20 mm
kecepatan target: normal saja tidak terlalu cepat karna ada box yang harus dicapit.

---

## [6] CLAUDE

*10/8/2026, 10:53:33 PM*

`ID: 01a11c37-e3e4-7414-a672-312778b3af1f`

Saya asumsikan "track width 3 cm" adalah **lebar garis lintasan** (30 mm) dan jarak antar roda belakang (T) adalah **100 mm**. Kalau maksud 3 cm itu hal lain, misalnya lebar roda, beri tahu saya.

## Hasil perhitungan

| Besaran | Nilai | Penilaian |
|---|---|---|
| L_sensor / T | 58,08 / 100 = **0,58** | Aman, di batas bawah rentang ideal 0,5-1,5. Stabil dan tidak mudah osilasi. |
| Lebar array (4 × 20 mm) | **80 mm** | Lebih kecil dari T (100 mm), jadi tidak melebar melewati roda. |
| Pitch 20 mm vs garis 30 mm | pitch < lebar garis | **Bagus**: garis selalu tertangkap minimal 1 sensor, sering 2. |
| Error maksimum terbaca | sekitar ±55 mm dari tengah | Sudut koreksi maksimum arctan(40/58,08) ≈ **34,5°** |
| Radius sapuan caster saat pivot | 92,73 mm | Lebih besar dari setengah T (50 mm), jadi area bebas harus dihitung dari caster. |

Kesimpulannya, kombinasi geometri dan sensor ini cocok untuk tikungan tajam pada kecepatan sedang.

## Posisi sensor dan error

Beri bobot posisi sensor dari kiri ke kanan: **-40, -20, 0, +20, +40 mm** (atau -2, -1, 0, +1, +2 jika dalam satuan pitch). Hitung error dengan rata-rata berbobot:

error = Σ(bobot × nilai sensor) / Σ(nilai sensor)

Jika semua sensor membaca putih (garis hilang), pakai error terakhir sebagai arah pencarian.

## Kecepatan

Karena harus mencapit box, target **0,15-0,25 m/s** masuk akal. Pada 0,2 m/s waktu antisipasi sensor = 58 / 200 ≈ **290 ms**, cukup longgar sehingga PID tidak perlu agresif. Kurangi lagi kecepatannya saat mendekati box.

## Nilai awal PID

Anggap PWM 0-255 dan error dalam satuan pitch (-2 sampai +2):

- **Base speed:** sekitar 100-130 (40-50% PWM)
- **Kp:** mulai dari **25-30**. Koreksi maksimum sekitar 50-60 PWM, atau setengah base speed.
- **Ki:** 0 (tidak perlu untuk line follower sederhana)
- **Kd:** mulai dari **Kp × 5 sampai 10**, tambahkan setelah Kp stabil.

Cara tuning:
1. Naikkan Kp sampai robot mulai bergoyang di jalur lurus, lalu turunkan ke sekitar 60% nilai itu.
2. Tambah Kd sampai goyangan hilang tanpa membuat robot "kaku" di tikungan.
3. Naikkan base speed bertahap sambil menguji ulang.

Nilai ini hanya titik awal. Hasil akhirnya tergantung berat baterai, gesekan roda, dan beban box.

## Tikungan 90° dan sudut tajam

Deteksi sudut ketika sensor kiri-tengah atau kanan-tengah menyala bersamaan (misalnya pola 11100 atau 00111). Setelah itu:
1. Maju **sekitar 55-60 mm** (L_sensor) supaya as roda tepat di titik sudut.
2. Pivot (roda berlawanan arah) sampai sensor tengah menemukan garis.
3. Lanjut ke PID normal.

Maju 58 mm pada 0,2 m/s memakan waktu sekitar 290 ms. Dengan encoder lebih akurat, tanpa encoder pakai `delay` hasil kalibrasi di lapangan.

## Catatan untuk gripper dan box

- Saat membawa box, titik berat bergeser ke depan. Pastikan massa box tidak membuat caster terlalu terbebani atau roda belakang kehilangan traksi.
- Lengan gripper menyapu lebih jauh dari caster (di gambar menjulur ke kanan), jadi hitung radius sapuan terjauhnya sebelum memutuskan jarak aman ke dinding atau box lain di track.
- Setelah mengangkat box, turunkan base speed 10-20% dan uji ulang tikungan, karena inersia berubah.

Jika Anda mau, kirim kode Arduino/ESP32 line follower Anda dan saya bisa bantu menyesuaikan pembacaan 5 sensor, logika sudut, dan PID-nya.

---

## [7] USER

*10/8/2026, 10:58:10 PM*

`ID: 01a11c3c-763c-7a18-a0d0-52df1b4c542d`

saya belum membuatkan kode nya, coba buatkan dengan menggunakan library AFMotor.h dan boardnya Arduino Uno, pada shield tersebut motor terhubung ke M3(kanan)  dan M4 (kiri), apakah bisa?, sensor S1 hingga S5 terhubng berurutan dari A0 ke A4 dan sensor IR box terhubung ke pin A5, lalu untuk buka box derajat yang diseting adalah 0 derajat dan untuk menutup diatas 90 derajat hingga 120 derajat. buatkan programnya dengan benar dan baik tanpa ada kendal , melalui perhitungan

---

## [8] CLAUDE

*10/8/2026, 10:59:10 PM*

`ID: 01a11c3c-763d-7d40-96f9-17c34a2437ad`

**Bisa.** Motor di M3 dan M4 didukung `AFMotor.h` (`AF_DCMotor(3)` dan `AF_DCMotor(4)`). Pin A0 sampai A5 tidak dipakai oleh shield V1, jadi bebas untuk 5 sensor garis dan sensor IR box. Servo gripper dipasang di header **SERVO_1 (pin 10)** shield.

## Perhitungan yang dipakai di program

| Parameter | Perhitungan | Nilai |
|---|---|---|
| Bobot sensor S1..S5 | pitch 20 mm, satuan error = 1 pitch | -2, -1, 0, +1, +2 |
| Koreksi maksimum | Kp × error maks = 25 × 2 | 50 PWM |
| Kecepatan roda | base 110 ± koreksi | 60 sampai 160 PWM |
| Maju sebelum pivot | L_sensor / v = 58,08 mm / 200 mm/s | **≈ 290 ms** (harus dikalibrasi) |
| Sudut gripper | buka 0°, tutup 110° | di dalam rentang 90° sampai 120° |

Catatan penting: 290 ms mengasumsikan kecepatan 200 mm/s pada PWM 100. Nilai ini **wajib dikalibrasi**: ukur berapa mm robot maju dalam 1 detik pada PWM tersebut, lalu `FORWARD_MS = 58,08 / kecepatan(mm/s) × 1000`.

## Program

<details>
<summary><strong>💻 Code Block (cpp) — 193 lines</strong></summary>

```cpp
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

// ================= SENSOR =================
const bool LINE_IS_HIGH = true;    // true: garis memberi nilai ADC tinggi. Ubah jika terbalik
const int  THRESHOLD    = 500;     // ambang ADC 0-1023, sesuaikan dengan hasil Serial
const int8_t WEIGHT[5]  = {-2, -1, 0, 1, 2};   // satuan pitch (20 mm)

const bool BOX_ACTIVE_LOW = true;  // modul IR umumnya LOW saat ada objek
const uint8_t BOX_DEBOUNCE = 3;

// ================= GRIPPER =================
const int GRIP_OPEN  = 0;
const int GRIP_CLOSE = 110;        // harus 90 s.d. 120
const int GRIP_STEP_DELAY = 15;    // ms per 2 derajat, gerak halus

// ================= PID =================
const float Kp = 25.0;
const float Kd = 120.0;            // sekitar Kp x 5, dihitung per siklus 10 ms
const int   BASE_SPEED_FREE  = 110;
const int   BASE_SPEED_CARRY = 90; // lebih pelan saat membawa box (turun sekitar 18%)
const int   MAX_CORRECTION   = 90;
const unsigned long LOOP_MS  = 10;

// ================= TIKUNGAN =================
const int  TURN_SPEED   = 110;
const int  ADVANCE_SPEED = 100;
const unsigned long FORWARD_MS   = 290;  // = 58,08 mm / kecepatan(mm/s), KALIBRASI!
const unsigned long MIN_PIVOT_MS = 120;  // waktu minimal agar sensor tengah lepas dari garis lama
const unsigned long PIVOT_TIMEOUT = 1500;
const int  SEARCH_SPEED = 90;

// ================= STATUS =================
float lastError = 0;
bool  holdingBox = false;
uint8_t boxCount = 0;
unsigned long lastLoop = 0;
bool s[5];

// ---------- fungsi motor ----------
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
    int v = analogRead(SENSOR_PIN[i]);
    s[i] = LINE_IS_HIGH ? (v > THRESHOLD) : (v < THRESHOLD);
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
  if (s[2]) { lastError = 0; return; }      // ada garis lurus (persimpangan), lanjut lurus

  // pivot: kiri => motor kiri mundur, kanan maju
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

  // --- deteksi box (dengan debounce) ---
  if (!holdingBox) {
    if (boxDetectedRaw()) boxCount++; else boxCount = 0;
    if (boxCount >= BOX_DEBOUNCE) {
      pickBox();
      return;
    }
  }

  // --- semua sensor kena garis (garis finish / garis lintang) ---
  if (count == 5) {
    if (holdingBox) releaseBoxAndStop();
    setMotor(ADVANCE_SPEED, ADVANCE_SPEED);        // belum membawa box: abaikan, terus maju
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

  int base = holdingBox ? BASE_SPEED_CARRY : BASE_SPEED_FREE;
  int left  = constrain(base + (int)correction, 0, 255);   // error + = garis di kanan, belok kanan
  int right = constrain(base - (int)correction, 0, 255);
  setMotor(left, right);
}
```

</details>

## Cara kerja singkat

1. Robot mengikuti garis dengan PID memakai 5 sensor.
2. Saat IR di A5 mendeteksi box (3 kali berturut-turut), robot berhenti dan gripper menutup perlahan dari 0° ke 110°.
3. Robot melanjutkan jalur dengan kecepatan lebih rendah (90 PWM).
4. Pada tikungan 90° (pola 11100 atau 00111), robot maju 290 ms agar as roda sampai di sudut, lalu pivot sampai sensor tengah menemukan garis.
5. Saat semua sensor kena garis dan robot sedang membawa box, gripper terbuka (0°), robot mundur sebentar lalu berhenti.

## Asumsi yang perlu Anda cek

- **Penempatan box:** program mengasumsikan box diambil saat terdeteksi lalu dilepas di garis lintang (11111). Kalau aturan lomba berbeda (misalnya "pindah ke kiri"), sebutkan, nanti logika lepasnya saya sesuaikan.
- **Logika sensor:** jika robot langsung belok ke arah salah, ubah `LINE_IS_HIGH`. Cek nilai ADC lewat `Serial.println(analogRead(A0))` di atas garis dan di atas lantai, lalu atur `THRESHOLD` di tengah keduanya.
- **Arah motor:** jika robot mundur saat seharusnya maju, tukar `DIR_FWD` dan `DIR_BWD`.
- **Servo SG90:** servo mengambil daya dari 5V Arduino. Jika Arduino reset saat gripper bergerak, beri catu daya servo terpisah (GND tetap disambung).
- **Gripper:** jika servo berdengung saat menjepit box, kurangi `GRIP_CLOSE` (misalnya 100°) agar tidak stall.
- **Tuning:** Kp/Kd adalah titik awal. Lakukan tuning bertahap seperti langkah sebelumnya, dan kalibrasi `FORWARD_MS` dulu sebelum menguji tikungan.

Apakah program ini sudah sesuai dan siap diubah ke format file `.ino`?

---

## [9] USER

*10/8/2026, 11:00:38 PM*

`ID: 01a11c3e-b7b9-7108-8c86-90fcf9c19b11`

perbaiki program, saya menggunakan sinyal output digital pada module tcrt5000 5 channel

---

## [10] CLAUDE

*10/8/2026, 11:01:05 PM*

`ID: 01a11c3e-b7b9-7af0-9a80-90009552eb80`

Dengan modul TCRT5000 5 channel output digital, pembacaan sensor memakai `digitalRead()`. Saya sudah menghapus `analogRead` dan `THRESHOLD`, karena ambang dibuat oleh potensiometer (LM393) di modul. Pin A0 sampai A5 tetap bisa dipakai sebagai pin digital di Uno.

## Perubahan

| Bagian | Sebelum | Sesudah |
|---|---|---|
| Pembacaan sensor | `analogRead` + threshold | `digitalRead` langsung |
| Setup pin | tidak ada | `pinMode(INPUT)` untuk A0 sampai A5 |
| Polaritas | `LINE_IS_HIGH` (ADC) | `LINE_LEVEL` (HIGH/LOW dari modul) |
| Kd | 120 | **60**, karena data digital berubah per langkah penuh (0,5 sampai 1 pitch), sehingga Kd yang besar membuat koreksi langsung jenuh |
| Debug | tidak ada | `DEBUG_SENSOR` mencetak pola sensor ke Serial |

## Program

<details>
<summary><strong>💻 Code Block (cpp) — 202 lines</strong></summary>

```cpp
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
```

</details>

## Langkah uji sebelum dijalankan penuh

1. Set `DEBUG_SENSOR true`, buka Serial Monitor (9600), lalu geser robot di atas garis. Pola harus berubah menjadi `00100` saat sensor tengah di garis, `10000` saat garis di paling kiri (S1), dan `00001` saat di paling kanan (S5).
2. Jika polanya terbalik (garis terbaca `0`, lantai `1`), ubah `LINE_LEVEL` ke `LOW`.
3. Jika urutan kiri-kanan terbalik, cek kabel S1 ke A0 sampai S5 ke A4.
4. Atur potensiometer pada modul agar LED sensor menyala/mati tepat pada perbedaan garis dan lantai, pada tinggi sensor 3 sampai 10 mm.
5. Setelah itu kembalikan `DEBUG_SENSOR false`, lalu kalibrasi `FORWARD_MS` dan tuning Kp/Kd.

Apakah program ini sudah sesuai dan siap diubah ke format file `.ino`?

---

