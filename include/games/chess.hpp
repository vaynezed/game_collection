#pragma once

#include "game.hpp"

class Opponent;
class Chess;

class Opponent {
public:
   Opponent(Chess* chess_ptr);
   Chess* chess_ptr;
   virtual void move() = 0;
};

class Chess : public Game {
public:
   enum class ChessPieceType : int8_t {
       black_cannon = 1, // ÅÚ
       black_elephant = 2, // Ïà
       black_guard = 3, // ÊË
       black_king = 4, // Ë§
       black_knight = 5, // Âí
       black_rook = 6, // ³µ
       black_soldier = 7, // ×ä
       red_cannon = 11,
       red_elephant = 12,
       red_guard = 13,
       red_king = 14,
       red_knight = 15,
       red_rook = 16,
       red_soldier = 17,
       empty = 0
   };

   void game_init(HWND& hwnd) override;
   void game_loop() override;
   void opponent_check_move();
   void check_game_data();
   void game_cleanup() override;
   void game_process_key_down(HWND hwnd, UINT message, WPARAM wParam,
       LPARAM lParam) override;
   std::wstring to_string() override;
   std::vector<std::wstring> game_models() override;
   void set_game_model(int idx) override;

   friend class Opponent;
   const ChessPieceType get_piece_type(const Pos& pos);
   void set_piece_type(const Pos& pos, ChessPieceType piece_type);

private:
   std::vector<std::vector<ChessPieceType>> chess_board;
   ULONGLONG g_tpre { 0 }, g_tnow { 0 };
   HBITMAP chess_bmp { nullptr };
   HBITMAP black_cannon { nullptr }, black_elephant { nullptr }, black_king { nullptr }, black_guard { nullptr }, black_knight { nullptr }, black_rook { nullptr }, black_soldier { nullptr };
   HBITMAP red_cannon { nullptr }, red_elephant { nullptr }, red_king { nullptr }, red_guard { nullptr }, red_knight { nullptr }, red_rook { nullptr }, red_soldier { nullptr };
   HBITMAP empty_chess { nullptr };
   HBITMAP* get_bmp_ptr_from_pos_type(ChessPieceType pos_type);
   HFONT hfont_new, hfont_old;
   int game_model_idx;
   std::unique_ptr<Opponent> opponent;

   std::vector<Pos> chess_target_pos_s;
   bool is_red_player;
   Pos selected_pos { -1, -1 };
   HDC g_hdc { nullptr }, g_mdc { nullptr }, g_bufdc { nullptr };
public:

   bool is_enemy(ChessPieceType lhs, ChessPieceType rhs);
   bool is_red(ChessPieceType piece_type);
   bool is_red(const Pos& pos);
   bool is_black(ChessPieceType piece_type);
   bool is_black(const Pos& pos);
   bool is_empty(ChessPieceType piece_type);
   bool is_empty(const Pos& pos);
   bool in_red_area(const Pos & pos);
   bool in_black_area(const Pos& pos);
   bool in_red_king_area(const Pos& pos);
   bool in_black_king_area(const Pos& pos);

   static inline const Pos NONE_POS { -1, -1 };

   static constexpr int chess_piece_height { 100 }, chess_piece_width { 100 };
   static constexpr int board_height { 10 }, board_width { 9 };
   static constexpr int board_bmp_width { 900 }, board_bmp_height { 1000 };
   static bool find_in_board(const std::vector<std::vector<ChessPieceType>>& chess_board, const std::vector<Pos>& pos, ChessPieceType target_type);
   static const Pos red_king_left_top_pos;
   static const Pos red_king_right_bottom_pos;
   static const Pos black_king_left_top_pos;
   static const Pos black_king_right_bottom_pos;
   static constexpr int text_height{ 200 };
   static constexpr int PAIR_MODEL{ 1 }, AI_MODEL{ 0 };

   void load_image();
   void load_image_data();
   void load_system_resource();
   void render();
   void render_chess_pieces();
   void render_chess_board();
   void render_empty_chess();
   void render_text();
   void process_click(int x_pos, int y_pos);
   void game_model_pair(int x_pos, int y_pos);
   void game_model_ai(int x_pos, int y_pos);
   void move_chess_piece(const Pos& source_pos, const Pos& target_pos);

   std::vector<Pos> cannon_move(int width_idx, int height_idx, ChessPieceType cannon_type);
   std::vector<Pos> rook_move(int width_idx, int height_idx, ChessPieceType rook_type);
   std::vector<Pos> knight_move(int width_idx, int height_idx, ChessPieceType knight_type);
   std::vector<Pos> elephant_move(int width_idx, int height_idx, ChessPieceType elephant_type);
   std::vector<Pos> guard_move(int width_idx, int height_idx, ChessPieceType guard_type);
   std::vector<Pos> king_move(int width_idx, int height_idx, ChessPieceType king_type);
   std::vector<Pos> soldier_move(int width_idx, int height_idx, ChessPieceType soldier_type);
   std::vector<Pos> black_player_pos(int chess_piece_width_idx, int chess_piece_height_idx, ChessPieceType chess_piece_type);
   std::vector<Pos> red_player_pos(int chess_piece_width_idx, int chess_piece_height_idx, ChessPieceType chess_piece_type);
   std::vector<Pos> chess_piece_pos(const Pos & pos);

   bool black_win();
   bool red_win();
   void game_end_dlg(bool is_red_win);
   static bool pos_in_board(Pos pos);
   static bool pos_in_board(int width_idx, int height_idx);
   static PosOffset get_knight_foot(PosOffset pos);
   static INT_PTR CALLBACK game_end_dlg_proc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
};

class AIOpponent : public Opponent {
public:
   AIOpponent(Chess* ptr, bool black_player) : Opponent(ptr), black_player(black_player) {}

   void move() override;
private:
	int evaluate_board();
   bool black_player;
   void make_move(const Pos& from, const Pos& to, Chess::ChessPieceType& captured_piece);
   void undo_move(const Pos& from, const Pos& to, Chess::ChessPieceType captured_piece);
   std::vector<std::pair<Pos, Pos>> get_all_moves(bool is_black);
   int minimax(int depth, bool is_maximizing);
};
