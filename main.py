import chess
import chess.engine
import chess.pgn
import math
import time
import random
from collections import defaultdict
import hashlib

class ChessEngine:
    def __init__(self):
        self.transposition_table = {}
        self.game_history = []
        self.nodes_searched = 0
        self.time_limit = 10  # 10 detik per gerakan
        self.start_time = 0
        
        # Tabel nilai posisi untuk setiap jenis bidak
        self.piece_square_tables = {
            chess.PAWN: [
                [0,  0,  0,  0,  0,  0,  0,  0],
                [50, 50, 50, 50, 50, 50, 50, 50],
                [10, 10, 20, 30, 30, 20, 10, 10],
                [5,  5, 10, 25, 25, 10,  5,  5],
                [0,  0,  0, 20, 20,  0,  0,  0],
                [5, -5,-10,  0,  0,-10, -5,  5],
                [5, 10, 10,-20,-20, 10, 10,  5],
                [0,  0,  0,  0,  0,  0,  0,  0]
            ],
            chess.KNIGHT: [
                [-50,-40,-30,-30,-30,-30,-40,-50],
                [-40,-20,  0,  0,  0,  0,-20,-40],
                [-30,  0, 10, 15, 15, 10,  0,-30],
                [-30,  5, 15, 20, 20, 15,  5,-30],
                [-30,  0, 15, 20, 20, 15,  0,-30],
                [-30,  5, 10, 15, 15, 10,  5,-30],
                [-40,-20,  0,  5,  5,  0,-20,-40],
                [-50,-40,-30,-30,-30,-30,-40,-50]
            ],
            chess.BISHOP: [
                [-20,-10,-10,-10,-10,-10,-10,-20],
                [-10,  0,  0,  0,  0,  0,  0,-10],
                [-10,  0,  5, 10, 10,  5,  0,-10],
                [-10,  5,  5, 10, 10,  5,  5,-10],
                [-10,  0, 10, 10, 10, 10,  0,-10],
                [-10, 10, 10, 10, 10, 10, 10,-10],
                [-10,  5,  0,  0,  0,  0,  5,-10],
                [-20,-10,-10,-10,-10,-10,-10,-20]
            ],
            chess.ROOK: [
                [0,  0,  0,  0,  0,  0,  0,  0],
                [5, 10, 10, 10, 10, 10, 10,  5],
                [-5,  0,  0,  0,  0,  0,  0, -5],
                [-5,  0,  0,  0,  0,  0,  0, -5],
                [-5,  0,  0,  0,  0,  0,  0, -5],
                [-5,  0,  0,  0,  0,  0,  0, -5],
                [-5,  0,  0,  0,  0,  0,  0, -5],
                [0,  0,  0,  5,  5,  0,  0,  0]
            ],
            chess.QUEEN: [
                [-20,-10,-10, -5, -5,-10,-10,-20],
                [-10,  0,  0,  0,  0,  0,  0,-10],
                [-10,  0,  5,  5,  5,  5,  0,-10],
                [-5,  0,  5,  5,  5,  5,  0, -5],
                [0,  0,  5,  5,  5,  5,  0, -5],
                [-10,  5,  5,  5,  5,  5,  0,-10],
                [-10,  0,  5,  0,  0,  0,  0,-10],
                [-20,-10,-10, -5, -5,-10,-10,-20]
            ],
            chess.KING: [
                [-30,-40,-40,-50,-50,-40,-40,-30],
                [-30,-40,-40,-50,-50,-40,-40,-30],
                [-30,-40,-40,-50,-50,-40,-40,-30],
                [-30,-40,-40,-50,-50,-40,-40,-30],
                [-20,-30,-30,-40,-40,-30,-30,-20],
                [-10,-20,-20,-20,-20,-20,-20,-10],
                [20, 20,  0,  0,  0,  0, 20, 20],
                [20, 30, 10,  0,  0, 10, 30, 20]
            ]
        }

    def get_piece_square_value(self, piece, square, endgame=False):
        """Mendapatkan nilai posisi untuk bidak tertentu"""
        piece_type = piece.piece_type
        color = piece.color
        
        if piece_type not in self.piece_square_tables:
            return 0
            
        # Konversi square ke koordinat baris dan kolom
        rank = chess.square_rank(square)
        file = chess.square_file(square)
        
        # Untuk bidak hitam, balik papan
        if color == chess.BLACK:
            rank = 7 - rank
            
        return self.piece_square_tables[piece_type][rank][file]

    def evaluate_board(self, board):
        """Fungsi evaluasi yang lebih canggih"""
        if board.is_checkmate():
            return -9999 if board.turn else 9999
        
        if board.is_stalemate() or board.is_insufficient_material():
            return 0

        # Nilai material dasar
        piece_values = {
            chess.PAWN: 100,
            chess.KNIGHT: 320,
            chess.BISHOP: 330,
            chess.ROOK: 500,
            chess.QUEEN: 900,
            chess.KING: 20000
        }

        score = 0
        white_pieces = 0
        black_pieces = 0

        # Hitung nilai material dan posisi
        for square in chess.SQUARES:
            piece = board.piece_at(square)
            if piece:
                piece_value = piece_values[piece.piece_type]
                position_value = self.get_piece_square_value(piece, square)
                
                if piece.color == chess.WHITE:
                    score += piece_value + position_value
                    white_pieces += 1
                else:
                    score -= piece_value + position_value
                    black_pieces += 1

        # Bonus mobilitas
        legal_moves = len(list(board.legal_moves))
        if board.turn == chess.WHITE:
            score += legal_moves * 2
        else:
            score -= legal_moves * 2

        # Evaluasi keamanan raja
        white_king_square = board.king(chess.WHITE)
        black_king_square = board.king(chess.BLACK)
        
        if white_king_square:
            white_king_safety = self.evaluate_king_safety(board, white_king_square, chess.WHITE)
            score += white_king_safety
            
        if black_king_square:
            black_king_safety = self.evaluate_king_safety(board, black_king_square, chess.BLACK)
            score -= black_king_safety

        # Bonus untuk bidak ganda dan terisolasi
        score += self.evaluate_pawn_structure(board)

        return score

    def evaluate_king_safety(self, board, king_square, color):
        """Evaluasi keamanan raja"""
        safety_score = 0
        
        # Periksa serangan pada kotak di sekitar raja
        king_vicinity = [
            king_square + offset for offset in [-9, -8, -7, -1, 1, 7, 8, 9]
            if 0 <= king_square + offset < 64 and 
            abs(chess.square_file(king_square) - chess.square_file(king_square + offset)) <= 1
        ]
        
        for square in king_vicinity:
            if 0 <= square < 64:
                if board.is_attacked_by(not color, square):
                    safety_score -= 10
                    
        return safety_score

    def evaluate_pawn_structure(self, board):
        """Evaluasi struktur pion"""
        score = 0
        
        white_pawns = board.pieces(chess.PAWN, chess.WHITE)
        black_pawns = board.pieces(chess.PAWN, chess.BLACK)
        
        # Hitung pion ganda dan terisolasi
        for file_idx in range(8):
            white_pawns_in_file = len([p for p in white_pawns if chess.square_file(p) == file_idx])
            black_pawns_in_file = len([p for p in black_pawns if chess.square_file(p) == file_idx])
            
            # Penalti untuk pion ganda
            if white_pawns_in_file > 1:
                score -= 10 * (white_pawns_in_file - 1)
            if black_pawns_in_file > 1:
                score += 10 * (black_pawns_in_file - 1)
                
        return score

    def order_moves(self, board, moves):
        """Urutkan gerakan untuk meningkatkan efisiensi alpha-beta"""
        def move_score(move):
            score = 0
            
            # Prioritaskan capture
            if board.is_capture(move):
                captured_piece = board.piece_at(move.to_square)
                moving_piece = board.piece_at(move.from_square)
                if captured_piece and moving_piece:
                    # MVV-LVA (Most Valuable Victim - Least Valuable Attacker)
                    score += 1000 + captured_piece.piece_type * 10 - moving_piece.piece_type
            
            # Prioritaskan promosi
            if move.promotion:
                score += 900
                
            # Prioritaskan cek
            board.push(move)
            if board.is_check():
                score += 50
            board.pop()
            
            return score
            
        return sorted(moves, key=move_score, reverse=True)

    def quiescence_search(self, board, alpha, beta, depth=0):
        """Pencarian quiescence untuk menangani posisi yang tidak stabil"""
        self.nodes_searched += 1
        
        if time.time() - self.start_time > self.time_limit:
            return self.evaluate_board(board)
            
        if depth > 10:  # Batasi kedalaman quiescence
            return self.evaluate_board(board)
            
        stand_pat = self.evaluate_board(board)
        
        if stand_pat >= beta:
            return beta
        if alpha < stand_pat:
            alpha = stand_pat
            
        # Hanya pertimbangkan capture dan promosi
        captures = [move for move in board.legal_moves if board.is_capture(move) or move.promotion]
        captures = self.order_moves(board, captures)
        
        for move in captures:
            board.push(move)
            score = -self.quiescence_search(board, -beta, -alpha, depth + 1)
            board.pop()
            
            if score >= beta:
                return beta
            if score > alpha:
                alpha = score
                
        return alpha

    def minimax(self, board, depth, alpha, beta, is_maximizing):
        """Algoritma minimax dengan alpha-beta pruning dan transposition table"""
        self.nodes_searched += 1
        
        # Cek batas waktu
        if time.time() - self.start_time > self.time_limit:
            return self.evaluate_board(board), None
            
        # Cek transposition table
        board_hash = str(board.fen())
        if board_hash in self.transposition_table:
            stored_depth, stored_score, stored_move = self.transposition_table[board_hash]
            if stored_depth >= depth:
                return stored_score, stored_move

        if depth == 0:
            return self.quiescence_search(board, alpha, beta), None

        if board.is_game_over():
            return self.evaluate_board(board), None

        best_move = None
        moves = list(board.legal_moves)
        moves = self.order_moves(board, moves)

        if is_maximizing:
            max_eval = -math.inf
            for move in moves:
                board.push(move)
                eval_score, _ = self.minimax(board, depth - 1, alpha, beta, False)
                board.pop()

                if eval_score > max_eval:
                    max_eval = eval_score
                    best_move = move

                alpha = max(alpha, eval_score)
                if beta <= alpha:
                    break  # Alpha-beta pruning
                    
            # Simpan ke transposition table
            self.transposition_table[board_hash] = (depth, max_eval, best_move)
            return max_eval, best_move
        else:
            min_eval = math.inf
            for move in moves:
                board.push(move)
                eval_score, _ = self.minimax(board, depth - 1, alpha, beta, True)
                board.pop()

                if eval_score < min_eval:
                    min_eval = eval_score
                    best_move = move

                beta = min(beta, eval_score)
                if beta <= alpha:
                    break  # Alpha-beta pruning
                    
            # Simpan ke transposition table
            self.transposition_table[board_hash] = (depth, min_eval, best_move)
            return min_eval, best_move

    def iterative_deepening(self, board, max_depth=6):
        """Iterative deepening untuk mendapatkan hasil terbaik dalam waktu terbatas"""
        best_move = None
        best_score = -math.inf if board.turn == chess.WHITE else math.inf
        
        for depth in range(1, max_depth + 1):
            if time.time() - self.start_time > self.time_limit * 0.8:
                break
                
            self.nodes_searched = 0
            current_time = time.time()
            
            score, move = self.minimax(board, depth, -math.inf, math.inf, board.turn == chess.WHITE)
            
            elapsed = time.time() - current_time
            
            if move:
                best_move = move
                best_score = score
                print(f"Depth {depth}: {move} (Score: {score:.2f}, Nodes: {self.nodes_searched}, Time: {elapsed:.2f}s)")
                
        return best_move, best_score

    def get_best_move(self, board):
        """Mendapatkan gerakan terbaik untuk posisi saat ini"""
        self.start_time = time.time()
        
        # Cek apakah ada gerakan yang dipaksa (checkmate dalam 1)
        for move in board.legal_moves:
            board.push(move)
            if board.is_checkmate():
                board.pop()
                return move
            board.pop()
            
        # Gunakan iterative deepening
        best_move, score = self.iterative_deepening(board)
        
        total_time = time.time() - self.start_time
        print(f"Total waktu berpikir: {total_time:.2f}s")
        print(f"Evaluasi akhir: {score:.2f}")
        
        return best_move

    def print_board_fancy(self, board):
        """Cetak papan dengan format yang lebih menarik"""
        print("\n  a b c d e f g h")
        print("  ─────────────────")
        
        for rank in range(7, -1, -1):
            line = f"{rank + 1}│"
            for file in range(8):
                square = chess.square(file, rank)
                piece = board.piece_at(square)
                
                if piece:
                    # Gunakan simbol Unicode untuk bidak
                    symbols = {
                        'P': '♙', 'N': '♘', 'B': '♗', 'R': '♖', 'Q': '♕', 'K': '♔',
                        'p': '♟', 'n': '♞', 'b': '♝', 'r': '♜', 'q': '♛', 'k': '♚'
                    }
                    symbol = symbols.get(piece.symbol(), piece.symbol())
                else:
                    symbol = '·'
                    
                line += f"{symbol} "
            print(line + f"│{rank + 1}")
            
        print("  ─────────────────")
        print("  a b c d e f g h\n")

    def save_game(self, board, filename="game.pgn"):
        """Simpan permainan ke file PGN"""
        game = chess.pgn.Game()
        game.headers["Event"] = "Chess Engine Game"
        game.headers["White"] = "Human"
        game.headers["Black"] = "Engine"
        game.headers["Result"] = board.result()
        
        node = game
        board_copy = chess.Board()
        
        for move_uci in self.game_history:
            move = chess.Move.from_uci(move_uci)
            node = node.add_variation(move)
            board_copy.push(move)
            
        with open(filename, "w") as f:
            f.write(str(game))
        print(f"Permainan disimpan ke {filename}")

def main():
    engine = ChessEngine()
    board = chess.Board()
    
    print("🏁 Selamat datang di Chess Engine!")
    print("Ketik 'quit' untuk keluar, 'undo' untuk membatalkan gerakan terakhir")
    print("Format gerakan: e2e4, Nf3, O-O, dll.\n")

    while not board.is_game_over():
        engine.print_board_fancy(board)
        
        # Tampilkan informasi status
        if board.is_check():
            print("⚠️  SKAK!")
        
        turn_name = "Putih (Anda)" if board.turn == chess.WHITE else "Hitam (Engine)"
        print(f"Giliran: {turn_name}")
        print(f"Gerakan legal: {len(list(board.legal_moves))}")
        
        if board.turn == chess.WHITE:
            # Giliran pemain
            while True:
                move_input = input("\nMasukkan gerakan Anda: ").strip()
                
                if move_input.lower() == 'quit':
                    print("Terima kasih telah bermain!")
                    return
                    
                if move_input.lower() == 'undo':
                    if len(engine.game_history) >= 2:
                        # Batalkan 2 gerakan terakhir (pemain dan engine)
                        board.pop()
                        board.pop()
                        engine.game_history = engine.game_history[:-2]
                        print("Gerakan dibatalkan!")
                        break
                    else:
                        print("Tidak ada gerakan yang bisa dibatalkan!")
                        continue

                if not move_input:
                    print("❌ Harap masukkan gerakan!")
                    continue

                try:
                    move = None
                    
                    # Coba parse sebagai UCI format dulu (e2e4)
                    if len(move_input) >= 4 and len(move_input) <= 5:
                        try:
                            move = chess.Move.from_uci(move_input)
                            # Debug print
                            print(f"Debug: Parsed UCI move: {move}")
                        except:
                            pass
                    
                    # Jika UCI gagal, coba SAN format (e4, Nf3, O-O)
                    if move is None:
                        try:
                            move = board.parse_san(move_input)
                            print(f"Debug: Parsed SAN move: {move}")
                        except:
                            pass
                    
                    if move is None:
                        print("❌ Format gerakan tidak valid! Gunakan format seperti: e2e4, e4, Nf3, O-O")
                        continue
                    
                    # Cek apakah gerakan legal
                    legal_moves = list(board.legal_moves)
                    print(f"Debug: Total legal moves: {len(legal_moves)}")
                    print(f"Debug: Trying move: {move}")
                    print(f"Debug: Move in legal moves: {move in legal_moves}")
                    
                    if move in legal_moves:
                        san_notation = board.san(move)  # Simpan notasi SAN sebelum push
                        board.push(move)
                        engine.game_history.append(move.uci())
                        print(f"✓ Gerakan Anda: {san_notation}")
                        break
                    else:
                        print("❌ Gerakan ilegal! Coba lagi.")
                        print("Gerakan legal yang tersedia:")
                        legal_sans = [board.san(m) for m in legal_moves]
                        print(", ".join(legal_sans[:10]))  # Tampilkan 10 gerakan pertama
                        if len(legal_sans) > 10:
                            print("... dan lainnya")
                        
                except Exception as e:
                    print(f"❌ Error saat memproses gerakan: {e}")
                    print(f"❌ Format gerakan tidak valid! Gunakan format seperti: e2e4, e4, Nf3, O-O")
                    
        else:
            # Giliran engine
            print("\n🤖 Engine sedang berpikir...")
            move = engine.get_best_move(board)
            
            if move:
                san_move = board.san(move)
                board.push(move)
                engine.game_history.append(move.uci())
                print(f"🎯 Engine memilih: {san_move}")
            else:
                print("❌ Engine tidak menemukan gerakan!")
                break

    # Permainan selesai
    engine.print_board_fancy(board)
    
    result = board.result()
    if result == "1-0":
        print("🎉 Putih menang!")
    elif result == "0-1":
        print("🎉 Hitam menang!")
    else:
        print("🤝 Permainan seri!")
        
    print(f"\nTotal gerakan: {len(engine.game_history)}")
    print("Riwayat gerakan:", " ".join(engine.game_history))
    
    # Tanyakan apakah ingin menyimpan permainan
    save = input("\nSimpan permainan ke file PGN? (y/n): ")
    if save.lower() == 'y':
        engine.save_game(board)

if __name__ == "__main__":
    main()