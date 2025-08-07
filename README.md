# Chess Engine C++

Sebuah chess engine sederhana yang dibuat dengan C++ dengan fitur analisis gerakan dan evaluasi performa pemain.

⚠️ **Status: Development** - Masih terdapat bug dan dalam tahap pengembangan aktif.

## Fitur

### Core Features

- ♟️ Board representation lengkap dengan Unicode pieces
- 🎯 Move generation untuk semua jenis bidak
- 🔍 Legal move validation
- 🏁 Deteksi checkmate dan stalemate
- 🤖 Engine dengan minimax algorithm + alpha-beta pruning

### Advanced Features

- 📊 **Move Analysis** - Setiap gerakan dianalisis dan diberi badge
- 🌳 **Search Tree Visualization** - Lihat proses berpikir engine
- ⚙️ **Configurable Settings** - Depth, time limit, dll
- 📈 **Performance Statistics** - Track akurasi dan improvement
- 💾 **Save Analysis** - Export analisis ke file

### Badge System

- 🔥 **Brilliant!!** - Gerakan luar biasa
- ⭐ **Best!** - Gerakan terbaik
- ✨ **Great** - Gerakan sangat baik
- ✓ **Good** - Gerakan baik
- ❓ **Inaccuracy** - Gerakan kurang tepat
- ❗ **Mistake** - Kesalahan
- 💥 **Blunder** - Kesalahan besar

## Setup & Installation

### Requirements

- C++ compiler dengan C++17 support (GCC, Clang, MSVC)
- Terminal yang support Unicode untuk tampilan board yang bagus

### Compile

```bash
g++ -O3 -std=c++17 -o chess_engine main.cpp
```

### Run

```bash
./chess_engine
```

## Cara Penggunaan

### Basic Commands

- `e2e4` - Masukkan gerakan (UCI format)
- `help` - Lihat gerakan legal yang tersedia
- `config` - Menu konfigurasi engine
- `info` - Lihat konfigurasi saat ini
- `analysis` - Lihat riwayat analisis gerakan
- `quit` - Keluar dari permainan

### Notasi Gerakan

Engine menggunakan **UCI (Universal Chess Interface)** format:

```
e2e4    - Pion dari e2 ke e4
g1f3    - Knight dari g1 ke f3
e1g1    - Castling kingside (as King e1 to g1)
e7e8q   - Pawn promotion to Queen
```

### Koordinat Board

```
  a b c d e f g h
8 ♜ ♞ ♝ ♛ ♚ ♝ ♞ ♜  8
7 ♟ ♟ ♟ ♟ ♟ ♟ ♟ ♟  7
6 · · · · · · · ·  6
5 · · · · · · · ·  5
4 · · · · · · · ·  4
3 · · · · · · · ·  3
2 ♙ ♙ ♙ ♙ ♙ ♙ ♙ ♙  2
1 ♖ ♘ ♗ ♕ ♔ ♗ ♘ ♖  1
  a b c d e f g h
```

## Configuration

### Menu Konfigurasi (`config`)

1. **Kedalaman maksimal** (1-15) - Kedalaman pencarian engine
2. **Batas waktu** (100-60000 ms) - Time limit per move
3. **Mode waktu** - Enable/disable time limit
4. **Tampilan tree** - Show/hide search tree
5. **Analisis gerakan** - Enable/disable move analysis
6. **Debug mode** - Show debug information
7. **Lihat riwayat** - View move analysis history
8. **Reset ke default** - Reset all settings

### Recommended Settings

**Untuk Bermain:**

```
Kedalaman: 5-6
Batas waktu: 5000 ms
Mode waktu: Aktif
Analisis gerakan: Aktif
```

**Untuk Belajar:**

```
Kedalaman: 3-4
Debug mode: Aktif
Tampilan tree: Aktif (untuk melihat proses berpikir)
```

## Contoh Gameplay

```bash
> e2e4

📊 ANALISIS GERAKAN:
═══════════════════════
Gerakan: e2e4
Badge: ⭐ Best!
Deskripsi: Gerakan terbaik dalam posisi ini
Ranking: #1 dari 20 gerakan
Score: 22 → 45 (+23)
═══════════════════════
✓ Gerakan Anda: e2e4

🤖 ENGINE BERPIKIR...
📊 DEPTH 5 SELESAI:
• Gerakan terbaik: e7e5
• Score: -25
• Nodes: 45623
• Waktu: 1234 ms
• Status: 🟡 Seimbang

🎯 Engine memilih: e7e5
```

## Known Issues & Bugs

⚠️ **Current Problems:**

- [ ] Evaluasi posisi kadang tidak akurat
- [ ] Badge system masih perlu fine-tuning
- [ ] Castling dan en passant belum fully implemented
- [ ] Promotion handling masih basic
- [ ] Performance bisa di-optimize lebih lanjut

⚠️ **Workarounds:**

- Jika evaluasi aneh, restart engine
- Untuk analysis yang akurat, set depth minimal 4
- Hindari posisi yang sangat complex (banyak piece)

## Development Roadmap

### Short Term

- [ ] Fix castling implementation
- [ ] Improve evaluation function
- [ ] Add en passant support
- [ ] Better move ordering

### Long Term

- [ ] Opening book integration
- [ ] Endgame tablebase
- [ ] UCI protocol support
- [ ] Multi-threading support
- [ ] Better position evaluation

## Performance

**Typical Performance (depth 5):**

- Nodes per second: 30K-50K NPS
- Time per move: 1-5 seconds
- Memory usage: ~10MB

**Estimated Playing Strength:**

- Depth 3-4: ~1200-1400 ELO
- Depth 5-6: ~1400-1600 ELO
- Depth 7+: ~1600+ ELO

## File Output

Engine dapat menyimpan analisis ke file:

```
chess_analysis_1234567890.txt
```

Format file berisi:

- Detail setiap gerakan dengan badge
- Statistik performa
- Score progression
- Ranking information

## Tips Penggunaan

### Untuk Pemain Pemula

1. Aktifkan analysis untuk belajar
2. Perhatikan badge setiap gerakan
3. Gunakan `help` untuk melihat gerakan legal
4. Set depth rendah (3-4) untuk response cepat

### Untuk Analysis

1. Aktifkan debug mode
2. Set depth tinggi (6-8)
3. Non-aktifkan time limit
4. Simpan analysis ke file untuk review

### Troubleshooting

- Jika engine crash: coba compile ulang
- Jika gerakan tidak accepted: cek format UCI
- Jika analysis aneh: restart dan coba lagi
- Jika lambat: kurangi depth atau aktifkan time limit

## Contributing

Contributions welcome! Masih banyak yang bisa diperbaiki:

1. Bug fixes
2. Performance optimization
3. Feature additions
4. Code cleanup
5. Documentation improvements

## License

This project is licensed under the **MIT License – Non-Commercial Edition**.  
Boleh digunakan untuk tujuan pribadi, edukasi, atau riset, tetapi **tidak boleh digunakan untuk tujuan komersial** tanpa izin tertulis.  
See the [LICENSE](LICENSE) file for full license details.

---

**Note:** Engine ini dibuat untuk tujuan educational dan experimental. Jangan expect performance level seperti Stockfish atau engine commercial lainnya!
