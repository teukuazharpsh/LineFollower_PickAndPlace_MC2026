# Robot Line Follower Pengangkut Boks (Proyek MC 2026)

Repositori ini berisi kode sumber untuk robot *line follower* berbasis Arduino Uno yang dilengkapi dengan lengan penjepit (*gripper*) untuk mengangkut boks. Robot ini menggunakan kontrol PID untuk pergerakan yang mulus, penanganan tikungan tajam 90 derajat, serta antarmuka mode tes melalui Serial Monitor.

## 📋 Fitur Utama
- **Kontrol Proporsional-Integral-Derivatif (PID):** Memastikan robot mengikuti garis dengan presisi menggunakan 5 kanal sensor digital.
- **Sistem Pengangkut Boks:** Menggunakan sensor inframerah (IR) *obstacle* untuk mendeteksi boks dan motor servo untuk menggerakkan penjepit.
- **Mode Tes Interaktif:** Antarmuka Serial Monitor bawaan untuk menguji motor, sensor, penjepit, dan melakukan kalibrasi sebelum berjalan di lintasan.
- **Penanganan Tikungan Tajam:** Algoritma khusus (`handleCorner`) untuk mendeteksi dan melewati tikungan bersudut 90 derajat secara akurat.

## 🛠️ Perangkat Keras (Hardware)
- **Mikrokontroler:** Arduino Uno
- **Driver Motor:** L293D Motor Drive Shield (menggunakan *library* `AFMotor.h`)
- **Sensor Garis:** Modul TCRT5000 5-Kanal (Output Digital)
- **Sensor Boks:** Modul Sensor IR Obstacle (Output Digital)
- **Aktuator:** 2x Motor DC (Penggerak utama) & 1x Motor Servo SG90 (Penjepit/Gripper)

## 🔌 Konfigurasi Pin
| Komponen | Pin Arduino / Shield | Deskripsi |
| :--- | :--- | :--- |
| **Motor Kanan** | M3 (Shield) | Dideklarasikan sebagai `motorR` |
| **Motor Kiri** | M4 (Shield) | Dideklarasikan sebagai `motorL` |
| **Servo Gripper**| Pin 10 (SERVO_1)| Dihubungkan ke *header* SERVO_1 pada Shield |
| **Sensor Garis** | A0, A1, A2, A3, A4 | S1 (Kiri Luar) hingga S5 (Kanan Luar) |
| **Sensor Boks** | A5 | Aktif LOW (Bisa disesuaikan pada variabel global) |

## 🚀 Panduan Penggunaan

### 1. Instalasi Library
Pastikan Anda telah memasang *library* berikut di Arduino IDE:
- `Adafruit Motor Shield library` (V1) - Untuk mengendalikan M3 dan M4.
- `Servo` - *Library* bawaan Arduino.

### 2. Memulai Program (Mode Tes)
Secara bawaan (*default*), program akan memulai dalam **Mode Tes** untuk alasan keamanan. 
1. Unggah program ke Arduino Uno.
2. Buka **Serial Monitor** pada Arduino IDE.
3. Atur *baud rate* ke **9600** dan pastikan opsi akhir baris diatur ke *No line ending* atau *Newline*.
4. Ketik perintah berikut pada Serial Monitor untuk melakukan pengujian:
   - `1` / `2` : Menguji masing-masing motor (kiri/kanan).
   - `3` / `4` : Menguji pergerakan maju/mundur.
   - `5` / `6` : Menguji putaran (*pivot*) di tempat.
   - `s` : Memantau pembacaan sensor garis dan boks secara aktual (*real-time*).
   - `g` : Menguji rentang gerak penjepit (buka-tutup).
   - `k` : Mengkalibrasi durasi maju saat di tikungan (`FORWARD_MS`).
   - `r` : **Menjalankan program utama (Robot akan hitung mundur dan mulai berjalan).**
   - `h` : Menampilkan kembali menu bantuan.

### 3. Penyesuaian (Tuning) Parameter
Jika robot berjalan kurang stabil, sesuaikan konstanta berikut pada kode program:
- `Kp` dan `Kd`: Parameter kendali PID. Atur secara proporsional sesuai dengan kecepatan dan respon motor Anda.
- `BASE_SPEED_FREE`: Kecepatan robot saat tidak membawa beban boks.
- `BASE_SPEED_CARRY`: Kecepatan robot saat membawa boks (dianjurkan lebih lambat agar stabil).
- `FORWARD_MS`: Waktu (dalam milidetik) yang dibutuhkan robot untuk melangkah maju sejauh titik poros as roda sebelum melakukan manuver *pivot* di tikungan tajam. Kalibrasi nilai ini menggunakan fitur `k` pada Mode Tes.