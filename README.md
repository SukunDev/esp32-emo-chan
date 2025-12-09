# ESP32 Emo Chan

Robot interaktif berbasis ESP32-C3 dengan ekspresi mata animasi pada OLED display. Robot ini dapat merespons interaksi pengguna melalui tombol dan menampilkan berbagai emosi serta pergerakan otomatis.

[![Preview](asset/preview.gif)](asset/preview.mp4)

## Deskripsi

ESP32 Emo Chan adalah proyek robot interaktif yang menggunakan ESP32-C3 sebagai mikrokontroler utama. Robot ini dilengkapi dengan:

- **OLED Display (SSD1306)** - Menampilkan mata robot dengan animasi yang halus dan ekspresif
- **Motor DC** - Menggerakkan robot ke berbagai arah (maju, mundur, kiri, kanan)
- **Buzzer** - Memutar melodi dan suara
- **Button** - Interface interaksi dengan pengguna

Robot memiliki sistem state machine yang kompleks dengan berbagai emosi dan perilaku yang dapat berubah berdasarkan interaksi pengguna atau waktu idle.

## Fitur

### Ekspresi & Emosi
- **Default** - Ekspresi normal dengan auto-blink
- **Happy** - Ekspresi senang dengan animasi tertawa
- **LongHappy** - Ekspresi senang yang lebih lama
- **Scared** - Ekspresi ketakutan dengan efek keringat
- **Scare** - Transisi dari scared
- **Curiosity** - Ekspresi penasaran dengan mata yang bergerak
- **Sleepy** - Ekspresi mengantuk
- **Asleep** - Robot tertidur
- **Angry** - Ekspresi marah

### Interaksi
- **Single Click** - Dapat memicu ekspresi happy atau angry secara acak
- **4x Click** - Memicu ekspresi scared
- **Long Press** - Memicu ekspresi long happy
- **Long Press Release** - Kembali ke ekspresi happy

### Perilaku Otomatis
- Pergerakan motor acak saat idle
- Transisi otomatis antar state berdasarkan waktu
- Auto-blink pada mata
- Animasi mata yang halus dan responsif

## Hardware Requirements

### Komponen Utama
- **ESP32-C3 DevKitM-1** - Mikrokontroler utama
- **OLED Display SSD1306** (128x64) - Display untuk mata robot
- **Motor Driver** (L298N atau sejenis) - Driver untuk motor DC
- **2x Motor DC** - Motor untuk pergerakan robot
- **Buzzer** - Speaker untuk suara
- **Button** - Tombol interaksi
- **Resistor** - Untuk pull-up button (jika diperlukan)

### Koneksi Pin

| Komponen | Pin ESP32-C3 |
|----------|--------------|
| I2C SDA  | GPIO 6       |
| I2C SCL  | GPIO 7       |
| Buzzer   | GPIO 8       |
| Button   | GPIO 10      |
| Motor IN1| GPIO 0       |
| Motor IN2| GPIO 1       |
| Motor IN3| GPIO 2       |
| Motor IN4| GPIO 3       |

## Instalasi

### Prasyarat
- [PlatformIO](https://platformio.org/) terinstall
- USB cable untuk upload dan monitoring

### Langkah Instalasi

1. **Clone repository**
```bash
git clone <repository-url>
cd pet_robot
```

2. **Install dependencies**
```bash
pio lib install
```

3. **Upload ke ESP32-C3**
```bash
pio run -t upload
```

4. **Monitor Serial** (opsional)
```bash
pio device monitor
```

## Struktur Project

```
pet_robot/
├── src/
│   ├── main.cpp              # Program utama
│   └── lib/
│       ├── RobotPet.h        # Class utama robot dengan state machine
│       ├── MotorManager.h     # Class untuk kontrol motor
│       ├── MotorManager.cpp
│       ├── ButtonManager.h   # Class untuk handle button input
│       ├── ButtonManager.cpp
│       ├── SoundPlayer.h     # Class untuk memutar melodi
│       └── FluxGarage_RoboEyes.h  # Library animasi mata robot
├── platformio.ini             # Konfigurasi PlatformIO
├── asset/
│   └── preview.mp4           # Video preview robot
└── README.md                 # Dokumentasi ini
```

## Penggunaan

### Setup Awal
1. Hubungkan semua komponen sesuai dengan pin mapping di atas
2. Upload program ke ESP32-C3
3. Robot akan otomatis memulai dengan melodi startup dan ekspresi default

### Interaksi dengan Robot

- **Klik sekali** - Robot akan merespons dengan ekspresi happy atau angry secara acak
- **Klik 4 kali cepat** - Robot akan menunjukkan ekspresi scared
- **Tekan dan tahan** - Robot akan menunjukkan ekspresi long happy
- **Lepas setelah tekan lama** - Robot kembali ke ekspresi happy

### Perilaku Otomatis
- Setelah 5 detik idle, robot akan masuk ke mode curiosity atau sleepy
- Robot akan bergerak secara acak setiap beberapa detik saat dalam mode default atau curiosity
- Setelah 10 detik tanpa interaksi, robot dapat tertidur

## Konfigurasi

Anda dapat mengubah berbagai parameter di file `src/lib/RobotPet.h`:

```cpp
static const unsigned long IDLE_DELAY = 5000;        // Delay sebelum idle (ms)
static const unsigned long DEEP_SLEEP_DELAY = 10000;  // Delay sebelum sleep (ms)
static const unsigned long ANGRY_DURATION = 5000;    // Durasi ekspresi angry (ms)
static const unsigned long HAPPY_DURATION = 500;      // Durasi ekspresi happy (ms)
```

## Library yang Digunakan

- **Adafruit SSD1306** (v2.5.15) - Driver untuk OLED display
- **Adafruit GFX** - Graphics library untuk drawing
- **FluxGarage RoboEyes** - Library animasi mata robot (included)

## Troubleshooting

### Display tidak muncul
- Periksa koneksi I2C (SDA/SCL)
- Pastikan alamat I2C display adalah 0x3C
- Jalankan I2C scanner untuk memverifikasi koneksi

### Motor tidak bergerak
- Periksa koneksi motor driver
- Pastikan power supply cukup untuk motor
- Verifikasi pin motor sudah terhubung dengan benar

### Button tidak responsif
- Periksa koneksi button ke GPIO 10
- Pastikan button menggunakan pull-up (INPUT_PULLUP)
- Cek debounce delay di ButtonManager

