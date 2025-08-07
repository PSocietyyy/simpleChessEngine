#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <chrono>
#include <climits>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <ctime>

using namespace std;
using namespace chrono;

// Enums untuk pieces dan colors
enum PieceType { PAWN = 1, KNIGHT = 2, BISHOP = 3, ROOK = 4, QUEEN = 5, KING = 6 };
enum Color { WHITE = 0, BLACK = 1 };

// Struktur untuk piece
struct Piece {
    PieceType type;
    Color color;
    
    Piece() : type(PieceType(0)), color(WHITE) {}
    Piece(PieceType t, Color c) : type(t), color(c) {}
    
    bool isEmpty() const { return type == 0; }
    
    char getSymbol() const {
        if (isEmpty()) return '.';
        
        char symbols[] = {'.', 'P', 'N', 'B', 'R', 'Q', 'K'};
        char symbol = symbols[type];
        return (color == BLACK) ? tolower(symbol) : symbol;
    }
    
    // Unicode symbols untuk display yang lebih cantik
    string getUnicodeSymbol() const {
        if (isEmpty()) return "·";
        
        string white_pieces[] = {"", "♙", "♘", "♗", "♖", "♕", "♔"};
        string black_pieces[] = {"", "♟", "♞", "♝", "♜", "♛", "♚"};
        
        return (color == WHITE) ? white_pieces[type] : black_pieces[type];
    }
};

// Struktur untuk move
struct Move {
    int from, to;
    PieceType promotion;
    bool isCapture;
    bool isCastling;
    bool isEnPassant;
    
    Move() : from(-1), to(-1), promotion(PieceType(0)), 
             isCapture(false), isCastling(false), isEnPassant(false) {}
             
    Move(int f, int t) : from(f), to(t), promotion(PieceType(0)), 
                         isCapture(false), isCastling(false), isEnPassant(false) {}
    
    bool isValid() const { return from >= 0 && to >= 0; }
    
    string toString() const {
        if (!isValid()) return "invalid";
        
        string result = "";
        result += char('a' + (from % 8));
        result += char('1' + (from / 8));
        result += char('a' + (to % 8));
        result += char('1' + (to / 8));
        
        if (promotion != 0) {
            char promo[] = {'.', '.', 'n', 'b', 'r', 'q', '.'};
            result += promo[promotion];
        }
        
        return result;
    }
};

class ChessBoard {
private:
    Piece board[64];
    Color currentPlayer;
    vector<string> moveHistory;
    int kingPositions[2]; // WHITE=0, BLACK=1
    bool castlingRights[4]; // KQkq
    int enPassantSquare;
    int halfMoveClock;
    int fullMoveNumber;

public:
    ChessBoard() {
        setupInitialPosition();
    }
    
    void setupInitialPosition() {
        // Clear board
        for (int i = 0; i < 64; i++) {
            board[i] = Piece();
        }
        
        // Setup pieces
        // White pieces
        board[0] = Piece(ROOK, WHITE);   board[7] = Piece(ROOK, WHITE);
        board[1] = Piece(KNIGHT, WHITE); board[6] = Piece(KNIGHT, WHITE);
        board[2] = Piece(BISHOP, WHITE); board[5] = Piece(BISHOP, WHITE);
        board[3] = Piece(QUEEN, WHITE);  board[4] = Piece(KING, WHITE);
        
        for (int i = 8; i < 16; i++) {
            board[i] = Piece(PAWN, WHITE);
        }
        
        // Black pieces
        board[56] = Piece(ROOK, BLACK);   board[63] = Piece(ROOK, BLACK);
        board[57] = Piece(KNIGHT, BLACK); board[62] = Piece(KNIGHT, BLACK);
        board[58] = Piece(BISHOP, BLACK); board[61] = Piece(BISHOP, BLACK);
        board[59] = Piece(QUEEN, BLACK);  board[60] = Piece(KING, BLACK);
        
        for (int i = 48; i < 56; i++) {
            board[i] = Piece(PAWN, BLACK);
        }
        
        currentPlayer = WHITE;
        kingPositions[WHITE] = 4;
        kingPositions[BLACK] = 60;
        
        for (int i = 0; i < 4; i++) castlingRights[i] = true;
        enPassantSquare = -1;
        halfMoveClock = 0;
        fullMoveNumber = 1;
    }
    
    void printBoard() const {
        cout << "\n  a b c d e f g h" << endl;
        cout << "  ─────────────────" << endl;
        
        for (int rank = 7; rank >= 0; rank--) {
            cout << (rank + 1) << "│";
            for (int file = 0; file < 8; file++) {
                int square = rank * 8 + file;
                cout << board[square].getUnicodeSymbol() << " ";
            }
            cout << "│" << (rank + 1) << endl;
        }
        
        cout << "  ─────────────────" << endl;
        cout << "  a b c d e f g h\n" << endl;
    }
    
    Piece getPiece(int square) const {
        if (square < 0 || square >= 64) return Piece();
        return board[square];
    }
    
    void setPiece(int square, const Piece& piece) {
        if (square >= 0 && square < 64) {
            board[square] = piece;
        }
    }
    
    Color getCurrentPlayer() const { return currentPlayer; }
    
    bool isSquareAttacked(int square, Color attackingColor) const {
        // Simplified attack detection
        for (int i = 0; i < 64; i++) {
            Piece piece = board[i];
            if (piece.isEmpty() || piece.color != attackingColor) continue;
            
            if (canPieceAttackSquare(i, square, piece)) {
                return true;
            }
        }
        return false;
    }
    
    bool canPieceAttackSquare(int from, int to, const Piece& piece) const {
        int dx = (to % 8) - (from % 8);
        int dy = (to / 8) - (from / 8);
        
        switch (piece.type) {
            case PAWN: {
                int direction = (piece.color == WHITE) ? 1 : -1;
                return abs(dx) == 1 && dy == direction;
            }
            case KNIGHT:
                return (abs(dx) == 2 && abs(dy) == 1) || (abs(dx) == 1 && abs(dy) == 2);
            case BISHOP:
                return abs(dx) == abs(dy) && isPathClear(from, to);
            case ROOK:
                return (dx == 0 || dy == 0) && isPathClear(from, to);
            case QUEEN:
                return (dx == 0 || dy == 0 || abs(dx) == abs(dy)) && isPathClear(from, to);
            case KING:
                return abs(dx) <= 1 && abs(dy) <= 1;
        }
        return false;
    }
    
    bool isPathClear(int from, int to) const {
        int dx = (to % 8) - (from % 8);
        int dy = (to / 8) - (from / 8);
        
        int stepX = (dx == 0) ? 0 : (dx > 0 ? 1 : -1);
        int stepY = (dy == 0) ? 0 : (dy > 0 ? 1 : -1);
        
        int current = from + stepX + stepY * 8;
        
        while (current != to) {
            if (!board[current].isEmpty()) return false;
            current += stepX + stepY * 8;
        }
        
        return true;
    }
    
    bool isInCheck(Color color) const {
        return isSquareAttacked(kingPositions[color], Color(1 - color));
    }
    
    vector<Move> generateLegalMoves() const {
        vector<Move> moves;
        
        for (int from = 0; from < 64; from++) {
            Piece piece = board[from];
            if (piece.isEmpty() || piece.color != currentPlayer) continue;
            
            vector<Move> pieceMoves = generatePieceMoves(from, piece);
            for (const Move& move : pieceMoves) {
                if (isMoveLegal(move)) {
                    moves.push_back(move);
                }
            }
        }
        
        return moves;
    }
    
    vector<Move> generatePieceMoves(int from, const Piece& piece) const {
        vector<Move> moves;
        
        switch (piece.type) {
            case PAWN:
                generatePawnMoves(from, piece, moves);
                break;
            case KNIGHT:
                generateKnightMoves(from, piece, moves);
                break;
            case BISHOP:
                generateBishopMoves(from, piece, moves);
                break;
            case ROOK:
                generateRookMoves(from, piece, moves);
                break;
            case QUEEN:
                generateQueenMoves(from, piece, moves);
                break;
            case KING:
                generateKingMoves(from, piece, moves);
                break;
        }
        
        return moves;
    }
    
    void generatePawnMoves(int from, const Piece& piece, vector<Move>& moves) const {
        int direction = (piece.color == WHITE) ? 1 : -1;
        int startRank = (piece.color == WHITE) ? 1 : 6;
        int rank = from / 8;
        int file = from % 8;
        
        // Forward move
        int to = from + direction * 8;
        if (to >= 0 && to < 64 && board[to].isEmpty()) {
            moves.push_back(Move(from, to));
            
            // Double pawn move
            if (rank == startRank) {
                to = from + direction * 16;
                if (to >= 0 && to < 64 && board[to].isEmpty()) {
                    moves.push_back(Move(from, to));
                }
            }
        }
        
        // Captures
        for (int df = -1; df <= 1; df += 2) {
            if (file + df >= 0 && file + df < 8) {
                to = from + direction * 8 + df;
                if (to >= 0 && to < 64) {
                    Piece target = board[to];
                    if (!target.isEmpty() && target.color != piece.color) {
                        Move move(from, to);
                        move.isCapture = true;
                        moves.push_back(move);
                    }
                }
            }
        }
    }
    
    void generateKnightMoves(int from, const Piece& piece, vector<Move>& moves) const {
        int knightMoves[] = {-17, -15, -10, -6, 6, 10, 15, 17};
        int file = from % 8;
        
        for (int delta : knightMoves) {
            int to = from + delta;
            int newFile = to % 8;
            
            if (to >= 0 && to < 64 && abs(file - newFile) <= 2) {
                Piece target = board[to];
                if (target.isEmpty() || target.color != piece.color) {
                    Move move(from, to);
                    move.isCapture = !target.isEmpty();
                    moves.push_back(move);
                }
            }
        }
    }
    
    void generateBishopMoves(int from, const Piece& piece, vector<Move>& moves) const {
        int directions[] = {-9, -7, 7, 9};
        
        for (int dir : directions) {
            int to = from + dir;
            while (to >= 0 && to < 64) {
                // Check for board edge
                if (abs((to % 8) - (from % 8)) != abs((to / 8) - (from / 8))) break;
                
                Piece target = board[to];
                if (target.isEmpty()) {
                    moves.push_back(Move(from, to));
                } else {
                    if (target.color != piece.color) {
                        Move move(from, to);
                        move.isCapture = true;
                        moves.push_back(move);
                    }
                    break;
                }
                to += dir;
            }
        }
    }
    
    void generateRookMoves(int from, const Piece& piece, vector<Move>& moves) const {
        int directions[] = {-8, -1, 1, 8};
        
        for (int dir : directions) {
            int to = from + dir;
            while (to >= 0 && to < 64) {
                // Check for board edge
                if (dir == -1 && to % 8 == 7) break;
                if (dir == 1 && to % 8 == 0) break;
                
                Piece target = board[to];
                if (target.isEmpty()) {
                    moves.push_back(Move(from, to));
                } else {
                    if (target.color != piece.color) {
                        Move move(from, to);
                        move.isCapture = true;
                        moves.push_back(move);
                    }
                    break;
                }
                to += dir;
            }
        }
    }
    
    void generateQueenMoves(int from, const Piece& piece, vector<Move>& moves) const {
        generateBishopMoves(from, piece, moves);
        generateRookMoves(from, piece, moves);
    }
    
    void generateKingMoves(int from, const Piece& piece, vector<Move>& moves) const {
        int directions[] = {-9, -8, -7, -1, 1, 7, 8, 9};
        
        for (int dir : directions) {
            int to = from + dir;
            if (to >= 0 && to < 64 && abs((to % 8) - (from % 8)) <= 1) {
                Piece target = board[to];
                if (target.isEmpty() || target.color != piece.color) {
                    Move move(from, to);
                    move.isCapture = !target.isEmpty();
                    moves.push_back(move);
                }
            }
        }
    }
    
    bool isMoveLegal(const Move& move) const {
        // Make a copy and test the move
        ChessBoard testBoard = *this;
        testBoard.makeMove(move);
        return !testBoard.isInCheck(currentPlayer);
    }
    
    bool makeMove(const Move& move) {
        if (!move.isValid()) return false;
        
        Piece movingPiece = board[move.from];
        Piece capturedPiece = board[move.to];
        
        // Update king position
        if (movingPiece.type == KING) {
            kingPositions[movingPiece.color] = move.to;
        }
        
        // Make the move
        board[move.to] = movingPiece;
        board[move.from] = Piece();
        
        // Switch players
        currentPlayer = Color(1 - currentPlayer);
        
        // Add to history
        moveHistory.push_back(move.toString());
        
        return true;
    }
    
    bool isGameOver() const {
        vector<Move> legalMoves = generateLegalMoves();
        return legalMoves.empty();
    }
    
    bool isCheckmate() const {
        return isInCheck(currentPlayer) && generateLegalMoves().empty();
    }
    
    bool isStalemate() const {
        return !isInCheck(currentPlayer) && generateLegalMoves().empty();
    }
    
    Move parseMove(const string& moveStr) const {
        if (moveStr.length() < 4) return Move();
        
        int fromFile = moveStr[0] - 'a';
        int fromRank = moveStr[1] - '1';
        int toFile = moveStr[2] - 'a';
        int toRank = moveStr[3] - '1';
        
        if (fromFile < 0 || fromFile > 7 || fromRank < 0 || fromRank > 7 ||
            toFile < 0 || toFile > 7 || toRank < 0 || toRank > 7) {
            return Move();
        }
        
        int from = fromRank * 8 + fromFile;
        int to = toRank * 8 + toFile;
        
        return Move(from, to);
    }
};

class ChessEngine {
private:
    unordered_map<string, pair<int, int>> transpositionTable;
    int nodesSearched;
    steady_clock::time_point startTime;
    int timeLimit; // in milliseconds
    int maxDepth;
    bool showTree;
    bool useTimeLimit;
    bool enableMoveAnalysis;
    
    int pieceValues[7] = {0, 100, 320, 330, 500, 900, 20000};
    
    // Move analysis history
    struct MoveAnalysis {
        Move move;
        string badge;
        string description;
        int scoreBefore;
        int scoreAfter;
        int scoreDiff;
        int rank; // Position in move ordering (1 = best)
        
        MoveAnalysis() : scoreBefore(0), scoreAfter(0), scoreDiff(0), rank(0) {}
    };
    
    vector<MoveAnalysis> moveHistory;
    int lastEvaluation;

public:
    ChessEngine() : nodesSearched(0), timeLimit(5000), maxDepth(5), showTree(false), 
                    useTimeLimit(true), enableMoveAnalysis(true), lastEvaluation(0) {}
    
    void showConfig() const {
        cout << "\n📊 KONFIGURASI ENGINE" << endl;
        cout << "═══════════════════════" << endl;
        cout << "• Kedalaman maksimal   : " << maxDepth << endl;
        cout << "• Batas waktu         : " << timeLimit << " ms" << endl;
        cout << "• Mode waktu          : " << (useTimeLimit ? "Aktif" : "Non-aktif") << endl;
        cout << "• Tampilkan tree      : " << (showTree ? "Ya" : "Tidak") << endl;
        cout << "• Analisis gerakan    : " << (enableMoveAnalysis ? "Aktif" : "Non-aktif") << endl;
        cout << "═══════════════════════\n" << endl;
    }
    
    void configure() {
        int choice;
        while (true) {
            cout << "\n⚙️  MENU KONFIGURASI" << endl;
            cout << "════════════════════" << endl;
            cout << "1. Ubah kedalaman maksimal (" << maxDepth << ")" << endl;
            cout << "2. Ubah batas waktu (" << timeLimit << " ms)" << endl;
            cout << "3. Toggle mode waktu (" << (useTimeLimit ? "Aktif" : "Non-aktif") << ")" << endl;
            cout << "4. Toggle tampilan tree (" << (showTree ? "Ya" : "Tidak") << ")" << endl;
            cout << "5. Toggle analisis gerakan (" << (enableMoveAnalysis ? "Aktif" : "Non-aktif") << ")" << endl;
            cout << "6. Lihat riwayat analisis" << endl;
            cout << "7. Reset ke default" << endl;
            cout << "8. Kembali ke permainan" << endl;
            cout << "════════════════════" << endl;
            cout << "Pilih (1-8): ";
            
            if (!(cin >> choice)) {
                cin.clear();
                cin.ignore(10000, '\n');
                cout << "❌ Input tidak valid!" << endl;
                continue;
            }
            cin.ignore(); // Clear newline
            
            switch (choice) {
                case 1: {
                    cout << "Masukkan kedalaman (1-15): ";
                    int newDepth;
                    if (cin >> newDepth && newDepth >= 1 && newDepth <= 15) {
                        maxDepth = newDepth;
                        cout << "✓ Kedalaman diubah ke " << maxDepth << endl;
                    } else {
                        cout << "❌ Kedalaman harus antara 1-15!" << endl;
                    }
                    cin.ignore();
                    break;
                }
                case 2: {
                    cout << "Masukkan batas waktu dalam ms (100-60000): ";
                    int newTime;
                    if (cin >> newTime && newTime >= 100 && newTime <= 60000) {
                        timeLimit = newTime;
                        cout << "✓ Batas waktu diubah ke " << timeLimit << " ms" << endl;
                    } else {
                        cout << "❌ Batas waktu harus antara 100-60000 ms!" << endl;
                    }
                    cin.ignore();
                    break;
                }
                case 3:
                    useTimeLimit = !useTimeLimit;
                    cout << "✓ Mode waktu: " << (useTimeLimit ? "Aktif" : "Non-aktif") << endl;
                    break;
                case 4:
                    showTree = !showTree;
                    cout << "✓ Tampilan tree: " << (showTree ? "Aktif" : "Non-aktif") << endl;
                    if (showTree) {
                        cout << "⚠️  Peringatan: Tree akan ditampilkan, output bisa sangat panjang!" << endl;
                    }
                    break;
                case 5:
                    enableMoveAnalysis = !enableMoveAnalysis;
                    cout << "✓ Analisis gerakan: " << (enableMoveAnalysis ? "Aktif" : "Non-aktif") << endl;
                    if (enableMoveAnalysis) {
                        cout << "📈 Gerakan Anda akan dianalisis dan diberi badge!" << endl;
                    }
                    break;
                case 6:
                    showMoveHistory();
                    break;
                case 7:
                    maxDepth = 5;
                    timeLimit = 5000;
                    useTimeLimit = true;
                    showTree = false;
                    enableMoveAnalysis = true;
                    cout << "✓ Konfigurasi di-reset ke default" << endl;
                    break;
                case 8:
                    return;
                default:
                    cout << "❌ Pilihan tidak valid!" << endl;
            }
        }
    }
    
    void showMoveHistory() const {
        if (moveHistory.empty()) {
            cout << "\n📋 Belum ada riwayat analisis gerakan." << endl;
            return;
        }
        
        cout << "\n📈 RIWAYAT ANALISIS GERAKAN" << endl;
        cout << "══════════════════════════════════════════════" << endl;
        cout << "No. | Gerakan | Badge        | Deskripsi" << endl;
        cout << "──────────────────────────────────────────────" << endl;
        
        for (size_t i = 0; i < moveHistory.size(); i++) {
            const MoveAnalysis& analysis = moveHistory[i];
            cout << setw(3) << (i + 1) << " | " 
                 << setw(7) << analysis.move.toString() << " | " 
                 << analysis.badge << setw(12 - analysis.badge.length()) << " | " 
                 << analysis.description << endl;
        }
        
        cout << "══════════════════════════════════════════════" << endl;
        
        // Statistik
        int brilliant = 0, best = 0, great = 0, good = 0, inaccuracy = 0, mistake = 0, blunder = 0;
        for (const auto& analysis : moveHistory) {
            if (analysis.badge.find("🔥") != string::npos) brilliant++;
            else if (analysis.badge.find("⭐") != string::npos) best++;
            else if (analysis.badge.find("✨") != string::npos) great++;
            else if (analysis.badge.find("✓") != string::npos) good++;
            else if (analysis.badge.find("❓") != string::npos) inaccuracy++;
            else if (analysis.badge.find("❗") != string::npos) mistake++;
            else if (analysis.badge.find("💥") != string::npos) blunder++;
        }
        
        cout << "\n📊 STATISTIK GERAKAN:" << endl;
        cout << "🔥 Brilliant: " << brilliant << "  ⭐ Best: " << best << "  ✨ Great: " << great << "  ✓ Good: " << good << endl;
        cout << "❓ Inaccuracy: " << inaccuracy << "  ❗ Mistake: " << mistake << "  💥 Blunder: " << blunder << endl;
        
        if (!moveHistory.empty()) {
            double accuracy = (double)(brilliant + best + great + good) / moveHistory.size() * 100;
            cout << "🎯 Akurasi: " << fixed << setprecision(1) << accuracy << "%" << endl;
            
            // Tips berdasarkan performa
            if (accuracy >= 90) {
                cout << "🏆 Performa luar biasa! Anda bermain seperti master!" << endl;
            } else if (accuracy >= 75) {
                cout << "🥇 Performa sangat baik! Terus tingkatkan!" << endl;
            } else if (accuracy >= 60) {
                cout << "🥈 Performa baik. Fokuslah pada mengurangi kesalahan." << endl;
            } else if (accuracy >= 40) {
                cout << "🥉 Ada ruang untuk perbaikan. Pertimbangkan gerakan lebih hati-hati." << endl;
            } else {
                cout << "📚 Latihan lebih banyak diperlukan. Pelajari prinsip dasar catur." << endl;
            }
            
            // Tips spesifik
            if (blunder > moveHistory.size() * 0.2) {
                cout << "💡 Tip: Periksa gerakan lawan sebelum bergerak untuk menghindari blunder." << endl;
            }
            if (mistake > moveHistory.size() * 0.3) {
                cout << "💡 Tip: Luangkan lebih banyak waktu untuk memikirkan konsekuensi gerakan." << endl;
            }
            if (brilliant > 0) {
                cout << "⭐ Hebat! Anda menemukan gerakan brilliant!" << endl;
            }
        }
    }
    
    string analyzeMoveQuality(int scoreDiff, int rank, int totalMoves, bool isCapture, bool isCheck) {
        string badge = "";
        string description = "";
        
        // Konversi score difference (negative berarti buruk untuk pemain)
        int centipawns = -scoreDiff; // Negatif karena scoreDiff adalah dari perspektif engine
        
        if (rank == 1 && totalMoves > 3 && (centipawns > 150 || (isCapture && centipawns > 50))) {
            // Brilliant: Gerakan terbaik DAN memberikan keuntungan signifikan
            badge = "🔥 Brilliant!!";
            description = "Gerakan luar biasa yang memberikan keuntungan besar!";
        }
        else if (rank == 1) {
            // Best move
            badge = "⭐ Best!";
            description = "Gerakan terbaik dalam posisi ini";
        }
        else if (rank <= 2 && centipawns >= -15) {
            // Great move (ranking 1-2 dan dalam 15 centipawn)
            badge = "✨ Great";
            description = "Gerakan sangat baik";
        }
        else if (rank <= 3 && centipawns >= -35) {
            // Good move (ranking 1-3 dan dalam 35 centipawn)
            badge = "✓ Good";
            description = "Gerakan baik";
        }
        else if (centipawns >= -80) {
            // Inaccuracy (kehilangan 35-80 centipawn)
            badge = "❓ Inaccuracy";
            description = "Gerakan kurang tepat (-" + to_string(-centipawns) + " cp)";
        }
        else if (centipawns >= -200) {
            // Mistake (kehilangan 80-200 centipawn)
            badge = "❗ Mistake";
            description = "Kesalahan (-" + to_string(-centipawns) + " cp)";
        }
        else {
            // Blunder (kehilangan >200 centipawn)
            badge = "💥 Blunder";
            description = "Kesalahan besar (-" + to_string(-centipawns) + " cp)";
        }
        
        // Tambahan untuk gerakan khusus
        if (isCapture && centipawns >= -50) {
            description += " [good capture]";
        } else if (isCapture && centipawns < -100) {
            description += " [bad capture]";
        }
        
        if (isCheck && centipawns >= -25) {
            description += " [effective check]";
        }
        
        // Tambahan untuk ranking
        if (rank > totalMoves * 0.8) {
            description += " [unusual choice]";
        }
        
        return badge + " | " + description;
    }
    
    int rankMove(const ChessBoard& board, const Move& playerMove) {
        vector<Move> allMoves = board.generateLegalMoves();
        vector<pair<int, Move>> moveScores;
        
        // Evaluasi semua gerakan
        for (const Move& move : allMoves) {
            ChessBoard testBoard = board;
            testBoard.makeMove(move);
            int score = -evaluateBoard(testBoard); // Negative karena perspektif berubah
            moveScores.push_back({score, move});
        }
        
        // Urutkan berdasarkan score (tertinggi dulu)
        sort(moveScores.begin(), moveScores.end(), [](const auto& a, const auto& b) {
            return a.first > b.first;
        });
        
        // Cari ranking gerakan pemain
        for (size_t i = 0; i < moveScores.size(); i++) {
            if (moveScores[i].second.from == playerMove.from && 
                moveScores[i].second.to == playerMove.to) {
                return i + 1; // Ranking mulai dari 1
            }
        }
        
        return allMoves.size(); // Jika tidak ditemukan, ranking terburuk
    }
    
    void analyzePlayerMove(ChessBoard& board, const Move& playerMove) {
        if (!enableMoveAnalysis) return;
        
        // Evaluasi posisi sebelum gerakan
        int scoreBefore = evaluateBoard(board);
        
        // Buat gerakan
        ChessBoard newBoard = board;
        newBoard.makeMove(playerMove);
        
        // Evaluasi posisi setelah gerakan
        int scoreAfter = -evaluateBoard(newBoard); // Negative karena perspektif berubah
        
        // Hitung score difference
        int scoreDiff = scoreAfter - scoreBefore;
        
        // Rank gerakan pemain
        int rank = rankMove(board, playerMove);
        vector<Move> allMoves = board.generateLegalMoves();
        
        // Cek properti gerakan
        Piece capturedPiece = board.getPiece(playerMove.to);
        bool isCapture = !capturedPiece.isEmpty();
        
        newBoard = board;
        newBoard.makeMove(playerMove);
        bool isCheck = newBoard.isInCheck(newBoard.getCurrentPlayer());
        
        // Analisis kualitas
        string analysis = analyzeMoveQuality(scoreDiff, rank, allMoves.size(), isCapture, isCheck);
        
        // Split badge dan description
        size_t pipePos = analysis.find(" | ");
        string badge = analysis.substr(0, pipePos);
        string description = analysis.substr(pipePos + 3);
        
        // Simpan analisis
        MoveAnalysis moveAnalysis;
        moveAnalysis.move = playerMove;
        moveAnalysis.badge = badge;
        moveAnalysis.description = description;
        moveAnalysis.scoreBefore = scoreBefore;
        moveAnalysis.scoreAfter = scoreAfter;
        moveAnalysis.scoreDiff = scoreDiff;
        moveAnalysis.rank = rank;
        
        moveHistory.push_back(moveAnalysis);
        
        // Tampilkan analisis
        cout << "\n📊 ANALISIS GERAKAN:" << endl;
        cout << "═══════════════════════" << endl;
        cout << "Gerakan: " << playerMove.toString() << endl;
        cout << "Badge: " << badge << endl;
        cout << "Deskripsi: " << description << endl;
        cout << "Ranking: #" << rank << " dari " << allMoves.size() << " gerakan" << endl;
        cout << "Score: " << scoreBefore << " → " << scoreAfter;
        if (scoreDiff > 0) cout << " (+" << scoreDiff << ")";
        else if (scoreDiff < 0) cout << " (" << scoreDiff << ")";
        cout << endl;
        cout << "═══════════════════════" << endl;
        
        // Update evaluasi terakhir
        lastEvaluation = scoreAfter;
    }
    
    string getIndent(int depth) const {
        string indent = "";
        for (int i = 0; i < depth; i++) {
            indent += (i == depth - 1) ? "├─ " : "│  ";
        }
        return indent;
    }
    
    void printTreeNode(int depth, const Move& move, int score, bool isMaximizing, 
                      int alpha, int beta, const string& nodeType = "") const {
        if (!showTree) return;
        
        string indent = getIndent(depth);
        string player = isMaximizing ? "MAX" : "MIN";
        string moveStr = move.isValid() ? move.toString() : "root";
        
        cout << indent << player << " d=" << depth << " | " << moveStr 
             << " | Score=" << score << " | α=" << alpha << " β=" << beta;
        
        if (!nodeType.empty()) {
            cout << " [" << nodeType << "]";
        }
        
        cout << endl;
    }
    
    int evaluateBoard(const ChessBoard& board) {
        if (board.isCheckmate()) {
            return (board.getCurrentPlayer() == WHITE) ? -9999 : 9999;
        }
        
        if (board.isStalemate()) {
            return 0;
        }
        
        int score = 0;
        int materialScore = 0;
        int mobilityScore = 0;
        
        // Material evaluation
        for (int square = 0; square < 64; square++) {
            Piece piece = board.getPiece(square);
            if (!piece.isEmpty()) {
                int value = pieceValues[piece.type];
                if (piece.color == WHITE) {
                    materialScore += value;
                } else {
                    materialScore -= value;
                }
            }
        }
        
        // Mobility bonus
        vector<Move> legalMoves = board.generateLegalMoves();
        mobilityScore = legalMoves.size() * 2;
        if (board.getCurrentPlayer() == BLACK) {
            mobilityScore = -mobilityScore;
        }
        
        score = materialScore + mobilityScore;
        
        return score;
    }
    
    pair<int, Move> minimax(ChessBoard& board, int depth, int alpha, int beta, 
                           bool isMaximizing, int currentDepth = 0, Move lastMove = Move()) {
        nodesSearched++;
        
        // Print tree node if enabled
        if (showTree && currentDepth > 0) {
            printTreeNode(currentDepth, lastMove, evaluateBoard(board), 
                         isMaximizing, alpha, beta);
        }
        
        // Time check (only if time limit is enabled)
        if (useTimeLimit) {
            auto now = steady_clock::now();
            if (duration_cast<milliseconds>(now - startTime).count() > timeLimit) {
                int score = evaluateBoard(board);
                if (showTree) {
                    printTreeNode(currentDepth, lastMove, score, isMaximizing, 
                                 alpha, beta, "TIME_CUTOFF");
                }
                return {score, Move()};
            }
        }
        
        if (depth == 0 || board.isGameOver()) {
            int score = evaluateBoard(board);
            if (showTree) {
                string nodeType = (depth == 0) ? "LEAF" : "TERMINAL";
                printTreeNode(currentDepth, lastMove, score, isMaximizing, 
                             alpha, beta, nodeType);
            }
            return {score, Move()};
        }
        
        Move bestMove;
        vector<Move> legalMoves = board.generateLegalMoves();
        
        // Move ordering: captures first
        sort(legalMoves.begin(), legalMoves.end(), [&](const Move& a, const Move& b) {
            return a.isCapture > b.isCapture;
        });
        
        if (isMaximizing) {
            int maxEval = INT_MIN;
            for (size_t i = 0; i < legalMoves.size(); i++) {
                const Move& move = legalMoves[i];
                ChessBoard newBoard = board;
                newBoard.makeMove(move);
                
                auto [eval, _] = minimax(newBoard, depth - 1, alpha, beta, false, 
                                       currentDepth + 1, move);
                
                if (eval > maxEval) {
                    maxEval = eval;
                    bestMove = move;
                }
                
                alpha = max(alpha, eval);
                
                if (beta <= alpha) {
                    if (showTree) {
                        printTreeNode(currentDepth + 1, move, eval, false, 
                                     alpha, beta, "BETA_CUTOFF");
                    }
                    break; // Alpha-beta pruning
                }
            }
            return {maxEval, bestMove};
        } else {
            int minEval = INT_MAX;
            for (size_t i = 0; i < legalMoves.size(); i++) {
                const Move& move = legalMoves[i];
                ChessBoard newBoard = board;
                newBoard.makeMove(move);
                
                auto [eval, _] = minimax(newBoard, depth - 1, alpha, beta, true, 
                                       currentDepth + 1, move);
                
                if (eval < minEval) {
                    minEval = eval;
                    bestMove = move;
                }
                
                beta = min(beta, eval);
                
                if (beta <= alpha) {
                    if (showTree) {
                        printTreeNode(currentDepth + 1, move, eval, true, 
                                     alpha, beta, "ALPHA_CUTOFF");
                    }
                    break; // Alpha-beta pruning
                }
            }
            return {minEval, bestMove};
        }
    }
    
    Move getBestMove(ChessBoard& board) {
        startTime = steady_clock::now();
        nodesSearched = 0;
        
        cout << "\n🤖 ENGINE BERPIKIR..." << endl;
        cout << "═══════════════════════" << endl;
        cout << "Kedalaman maksimal: " << maxDepth << endl;
        cout << "Batas waktu: " << (useTimeLimit ? to_string(timeLimit) + " ms" : "Tidak terbatas") << endl;
        cout << "Tampilkan tree: " << (showTree ? "Ya" : "Tidak") << endl;
        cout << "═══════════════════════" << endl;
        
        Move bestMove;
        int bestScore = (board.getCurrentPlayer() == WHITE) ? INT_MIN : INT_MAX;
        
        if (showTree) {
            cout << "\n🌳 SEARCH TREE:" << endl;
            cout << "ROOT - Giliran: " << (board.getCurrentPlayer() == WHITE ? "WHITE" : "BLACK") << endl;
        }
        
        // Iterative deepening
        for (int depth = 1; depth <= maxDepth; depth++) {
            auto start = steady_clock::now();
            int prevNodesSearched = nodesSearched;
            nodesSearched = 0;
            
            if (showTree) {
                cout << "\n--- DEPTH " << depth << " ---" << endl;
            }
            
            auto [score, move] = minimax(board, depth, INT_MIN, INT_MAX, 
                                       board.getCurrentPlayer() == WHITE);
            
            auto elapsed = duration_cast<milliseconds>(steady_clock::now() - start);
            int currentDepthNodes = nodesSearched;
            nodesSearched += prevNodesSearched;
            
            if (move.isValid()) {
                bestMove = move;
                bestScore = score;
                
                cout << "\n📊 DEPTH " << depth << " SELESAI:" << endl;
                cout << "• Gerakan terbaik: " << move.toString() << endl;
                cout << "• Score: " << score << endl;
                cout << "• Nodes: " << currentDepthNodes << " (Total: " << nodesSearched << ")" << endl;
                cout << "• Waktu: " << elapsed.count() << " ms" << endl;
                cout << "• NPS: " << (elapsed.count() > 0 ? (currentDepthNodes * 1000 / elapsed.count()) : 0) << " nodes/s" << endl;
                
                // Evaluasi kualitas gerakan
                if (score > 500) cout << "• Status: 🟢 Sangat menguntungkan!" << endl;
                else if (score > 100) cout << "• Status: 🔵 Menguntungkan" << endl;
                else if (score > -100) cout << "• Status: 🟡 Seimbang" << endl;
                else if (score > -500) cout << "• Status: 🟠 Kurang menguntungkan" << endl;
                else cout << "• Status: 🔴 Berbahaya!" << endl;
            }
            
            // Break if we're running out of time (only if time limit is enabled)
            if (useTimeLimit && elapsed.count() > timeLimit * 0.8) {
                cout << "⏰ Batas waktu hampir habis, menghentikan pencarian..." << endl;
                break;
            }
        }
        
        auto totalTime = duration_cast<milliseconds>(steady_clock::now() - startTime);
        
        cout << "\n🏁 PENCARIAN SELESAI:" << endl;
        cout << "═══════════════════════" << endl;
        cout << "• Total waktu: " << totalTime.count() << " ms" << endl;
        cout << "• Total nodes: " << nodesSearched << endl;
        cout << "• Average NPS: " << (totalTime.count() > 0 ? (nodesSearched * 1000 / totalTime.count()) : 0) << " nodes/s" << endl;
        cout << "• Kedalaman tercapai: " << maxDepth << endl;
        cout << "• Evaluasi akhir: " << bestScore << endl;
        cout << "═══════════════════════" << endl;
        
        return bestMove;
    }
    
    bool isAnalysisEnabled() const {
        return enableMoveAnalysis;
    }
    
    void saveAnalysisToFile() const {
        if (moveHistory.empty()) {
            cout << "❌ Tidak ada data analisis untuk disimpan." << endl;
            return;
        }
        
        // Generate filename dengan timestamp
        auto now = chrono::system_clock::now();
        auto time_t = chrono::system_clock::to_time_t(now);
        stringstream ss;
        ss << "chess_analysis_" << time_t << ".txt";
        string filename = ss.str();
        
        ofstream file(filename);
        if (!file.is_open()) {
            cout << "❌ Gagal membuat file analisis." << endl;
            return;
        }
        
        file << "CHESS ENGINE - ANALISIS GERAKAN" << endl;
        file << "===============================" << endl;
        file << "Tanggal: " << ctime(&time_t);
        file << "Total gerakan: " << moveHistory.size() << endl << endl;
        
        file << "DETAIL GERAKAN:" << endl;
        file << "===============" << endl;
        for (size_t i = 0; i < moveHistory.size(); i++) {
            const MoveAnalysis& analysis = moveHistory[i];
            file << (i + 1) << ". " << analysis.move.toString() 
                 << " - " << analysis.badge << endl;
            file << "   " << analysis.description << endl;
            file << "   Score: " << analysis.scoreBefore << " → " << analysis.scoreAfter;
            if (analysis.scoreDiff != 0) {
                file << " (" << (analysis.scoreDiff > 0 ? "+" : "") << analysis.scoreDiff << ")";
            }
            file << ", Rank: #" << analysis.rank << endl << endl;
        }
        
        // Statistik
        int brilliant = 0, best = 0, great = 0, good = 0, inaccuracy = 0, mistake = 0, blunder = 0;
        for (const auto& analysis : moveHistory) {
            if (analysis.badge.find("🔥") != string::npos) brilliant++;
            else if (analysis.badge.find("⭐") != string::npos) best++;
            else if (analysis.badge.find("✨") != string::npos) great++;
            else if (analysis.badge.find("✓") != string::npos) good++;
            else if (analysis.badge.find("❓") != string::npos) inaccuracy++;
            else if (analysis.badge.find("❗") != string::npos) mistake++;
            else if (analysis.badge.find("💥") != string::npos) blunder++;
        }
        
        file << "STATISTIK:" << endl;
        file << "==========" << endl;
        file << "Brilliant: " << brilliant << endl;
        file << "Best: " << best << endl;
        file << "Great: " << great << endl;
        file << "Good: " << good << endl;
        file << "Inaccuracy: " << inaccuracy << endl;
        file << "Mistake: " << mistake << endl;
        file << "Blunder: " << blunder << endl;
        
        double accuracy = (double)(brilliant + best + great + good) / moveHistory.size() * 100;
        file << "Akurasi: " << fixed << setprecision(1) << accuracy << "%" << endl;
        
        file.close();
        cout << "✓ Analisis disimpan ke: " << filename << endl;
    }
};

int main() {
    ChessBoard board;
    ChessEngine engine;
    
    cout << "🏁 Selamat datang di Chess Engine C++ Advanced!" << endl;
    cout << "═══════════════════════════════════════════════" << endl;
    cout << "PERINTAH YANG TERSEDIA:" << endl;
    cout << "• e2e4, d2d4, dll.    - Gerakan catur" << endl;
    cout << "• 'config'            - Menu konfigurasi engine" << endl;
    cout << "• 'info'              - Lihat konfigurasi saat ini" << endl;
    cout << "• 'analysis'          - Lihat riwayat analisis gerakan" << endl;
    cout << "• 'help'              - Bantuan gerakan legal" << endl;
    cout << "• 'quit'              - Keluar dari permainan" << endl;
    cout << "═══════════════════════════════════════════════" << endl;
    cout << "🎯 FITUR ANALISIS GERAKAN AKTIF!" << endl;
    cout << "Setiap gerakan Anda akan dianalisis dan diberi badge:" << endl;
    cout << "🔥 Brilliant  ⭐ Best  ✨ Great  ✓ Good  ❓ Inaccuracy  ❗ Mistake  💥 Blunder" << endl;
    cout << "═══════════════════════════════════════════════" << endl;
    
    // Tampilkan konfigurasi awal
    engine.showConfig();
    
    while (!board.isGameOver()) {
        board.printBoard();
        
        if (board.isInCheck(board.getCurrentPlayer())) {
            cout << "⚠️  SKAK!" << endl;
        }
        
        string playerName = (board.getCurrentPlayer() == WHITE) ? "Putih (Anda)" : "Hitam (Engine)";
        cout << "Giliran: " << playerName << endl;
        
        vector<Move> legalMoves = board.generateLegalMoves();
        cout << "Gerakan legal: " << legalMoves.size() << endl;
        
        if (board.getCurrentPlayer() == WHITE) {
            // Player turn
            string input;
            while (true) {
                cout << "\n> ";
                getline(cin, input);
                
                // Convert to lowercase for command checking
                string command = input;
                transform(command.begin(), command.end(), command.begin(), ::tolower);
                
                if (command == "quit") {
                    cout << "Terima kasih telah bermain!" << endl;
                    return 0;
                }
                
                if (command == "config") {
                    engine.configure();
                    continue;
                }
                
                if (command == "info") {
                    engine.showConfig();
                    continue;
                }
                
                if (command == "analysis") {
                    engine.showMoveHistory();
                    continue;
                }
                
                if (command == "help") {
                    cout << "📋 GERAKAN LEGAL YANG TERSEDIA:" << endl;
                    cout << "══════════════════════════════" << endl;
                    for (size_t i = 0; i < legalMoves.size(); i++) {
                        cout << legalMoves[i].toString();
                        if (legalMoves[i].isCapture) cout << " (capture)";
                        if ((i + 1) % 8 == 0) cout << endl;
                        else cout << "  ";
                    }
                    if (legalMoves.size() % 8 != 0) cout << endl;
                    cout << "══════════════════════════════" << endl;
                    continue;
                }
                
                // Try to parse as move
                Move move = board.parseMove(input);
                if (!move.isValid()) {
                    cout << "❌ Format tidak valid! Gunakan: e2e4, atau ketik 'help' untuk bantuan" << endl;
                    continue;
                }
                
                // Check if move is legal
                bool isLegal = false;
                for (const Move& legalMove : legalMoves) {
                    if (legalMove.from == move.from && legalMove.to == move.to) {
                        isLegal = true;
                        break;
                    }
                }
                
                if (isLegal) {
                    // Analisis gerakan pemain sebelum membuatnya
                    if (engine.isAnalysisEnabled()) {
                        engine.analyzePlayerMove(board, move);
                    }
                    
                    board.makeMove(move);
                    cout << "✓ Gerakan Anda: " << move.toString() << endl;
                    break;
                } else {
                    cout << "❌ Gerakan ilegal!" << endl;
                    cout << "Contoh gerakan legal: ";
                    for (size_t i = 0; i < min(size_t(5), legalMoves.size()); i++) {
                        cout << legalMoves[i].toString() << " ";
                    }
                    cout << "\nKetik 'help' untuk melihat semua gerakan legal." << endl;
                }
            }
        } else {
            // Engine turn
            Move bestMove = engine.getBestMove(board);
            if (bestMove.isValid()) {
                board.makeMove(bestMove);
                cout << "\n🎯 Engine memilih: " << bestMove.toString() << endl;
                cout << "Tekan Enter untuk melanjutkan...";
                cin.get(); // Pause agar user bisa melihat analisis
            } else {
                cout << "❌ Engine tidak menemukan gerakan!" << endl;
                break;
            }
        }
    }
    
    // Game over
    board.printBoard();
    
    if (board.isCheckmate()) {
        Color winner = Color(1 - board.getCurrentPlayer());
        cout << "\n🎉 " << (winner == WHITE ? "Putih" : "Hitam") << " menang!" << endl;
    } else if (board.isStalemate()) {
        cout << "\n🤝 Permainan seri!" << endl;
    }
    
    // Tampilkan ringkasan analisis gerakan
    if (engine.isAnalysisEnabled()) {
        cout << "\n🎯 RINGKASAN PERFORMA ANDA:" << endl;
        engine.showMoveHistory();
        
        cout << "\nIngin menyimpan analisis ke file? (y/n): ";
        string saveChoice;
        getline(cin, saveChoice);
        if (saveChoice == "y" || saveChoice == "Y") {
            engine.saveAnalysisToFile();
        }
    }
    
    cout << "\nTerima kasih telah bermain!" << endl;
    return 0;
}