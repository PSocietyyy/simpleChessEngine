Berikut adalah dokumentasi terstruktur untuk menyusun **alur pembuatan engine catur tanpa neural network**, menggunakan pendekatan klasik seperti **alpha-beta pruning** dan **evaluasi posisi**:

---

## 📘 Dokumentasi Alur Pembuatan Engine Catur Tanpa Neural Network

### 📌 1. Tujuan

Membangun engine catur klasik yang dapat:

- Menerima posisi catur sebagai input (format FEN)
- Mengevaluasi posisi secara efisien menggunakan algoritma pencarian (alpha-beta)
- Menghasilkan langkah terbaik berdasarkan evaluasi

---

### 📌 2. Struktur Umum Engine

```text
[Input FEN]
     ↓
[Iterative Deepening Loop]
     ↓
[Alpha-Beta Search]
     ↓
[Quiescence Search] ─┐
     ↓               │
[Evaluasi Posisi] ◄──┘
     ↓
[Langkah Terbaik]
```

---

### 📌 2. Penjelasan Komponen

#### 🧩 a. FEN (Forsyth–Edwards Notation)

- FEN adalah format teks yang menggambarkan posisi papan saat ini.
- Contoh:
  `"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"`
  Ini menunjukkan papan awal.

#### 🧩 b. Iterative Deepening

- Melakukan pencarian dari kedalaman 1, lalu 2, dst, hingga waktu habis.
- Keuntungan:

  - Memberi hasil terbaik dalam batas waktu
  - Memperbaiki move ordering pada kedalaman berikutnya

#### 🧩 c. Alpha-Beta Pruning

- Versi optimasi dari minimax yang “memangkas” cabang tidak berguna.
- Menyimpan nilai α (nilai terbaik dari MAX) dan β (nilai terbaik dari MIN).
- Jika posisi lebih buruk dari β untuk pemain MIN, maka cabang dihentikan (karena lawan tidak akan memilihnya).

#### 🧩 d. Move Ordering

- Mengurutkan langkah agar langkah bagus diperiksa lebih dulu (membantu pruning).
- Contoh:

  - Coba langkah yang memakan bidak dulu
  - Coba langkah yang memberi skak lebih dulu

#### 🧩 e. Quiescence Search

- Memperpanjang pencarian jika posisi "berisik" (misal ada bidak bisa dimakan).
- Menghindari “horizon effect” dengan terus mengevaluasi hingga posisi “tenang”.

#### 🧩 f. Evaluasi Posisi

- Fungsi untuk menilai siapa yang unggul di posisi saat ini.
- Biasanya berbasis:

  - Nilai material (bidak, kuda, benteng, dll)
  - Posisi (kontrol pusat, mobilitas, pion ganda, dll)

#### 🧩 g. Output Langkah Terbaik

- Setelah pencarian selesai, langkah dengan skor tertinggi dipilih dan dikembalikan.

---

### 📌 3. Alur Pseudocode Umum

```python
def search_best_move(fen, time_limit):
    position = parse_fen(fen)
    best_move = None
    depth = 1
    start_time = now()

    while time_not_exceeded(start_time, time_limit):
        move, score = alpha_beta(position, depth, -inf, +inf, True)
        best_move = move
        depth += 1

    return best_move
```

---

### 📌 4. Evaluasi Posisi (Contoh Sederhana)

```python
def evaluate(position):
    piece_values = {
        'P': 100, 'N': 320, 'B': 330, 'R': 500, 'Q': 900, 'K': 20000,
        'p': -100, 'n': -320, 'b': -330, 'r': -500, 'q': -900, 'k': -20000
    }

    score = 0
    for square in position.board:
        if square in piece_values:
            score += piece_values[square]

    return score
```

---

### 📌 5. Visualisasi Iterative Deepening

```text
Depth 1  → Best move: e2e4 (eval: +20)
Depth 2  → Best move: e2e4 (eval: +35)
Depth 3  → Best move: e2e4 (eval: +40)
Depth 4  → Best move: d2d4 (eval: +60)
...
(time habis)
Final best move: d2d4
```

---

### 📌 6. Tahapan Pengembangan

| Tahap | Langkah                             |
| ----- | ----------------------------------- |
| 1     | Membuat parser FEN                  |
| 2     | Menghasilkan semua legal moves      |
| 3     | Implementasi fungsi evaluasi posisi |
| 4     | Implementasi minimax + alpha-beta   |
| 5     | Tambahkan iterative deepening       |
| 6     | Tambahkan quiescence search         |
| 7     | Tambahkan move ordering             |
| 8     | Tambahkan pengatur waktu (timer)    |
