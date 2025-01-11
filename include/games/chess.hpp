#pragma once

#include "game.hpp"

class Chess : public Game {
public:
    enum class ChessPieceType : int8_t {
        black_cannon = 1, // ÅÚ
        black_elephant = 2, // Ïà
        black_guard = 3, // ÊË
        black_king = 4, // Ë§
        black_knight = 5, // Âí
        black_rook = 6, //  ³µ
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
    void check_game_data();
    void game_cleanup() override;
    void game_process_key_down(HWND hwnd, UINT message, WPARAM wParam,
        LPARAM lParam) override;
    std::wstring to_string() override;

private:
    std::vector<std::vector<ChessPieceType>> chess_board;
    ULONGLONG g_tpre { 0 }, g_tnow { 0 };
    HBITMAP chess_bmp { nullptr };
    HBITMAP black_cannon { nullptr }, black_elephant { nullptr }, black_king { nullptr }, black_guard { nullptr }, black_knight { nullptr }, black_rook { nullptr }, black_soldier { nullptr };
    HBITMAP red_cannon { nullptr }, red_elephant { nullptr }, red_king { nullptr }, red_guard { nullptr }, red_knight { nullptr }, red_rook { nullptr }, red_soldier { nullptr };
    HBITMAP empty_chess { nullptr };
    HBITMAP* get_bmp_ptr_from_pos_type(ChessPieceType pos_type);

    std::vector<Pos> chess_target_pos_s;
    bool is_red_player;

    bool is_enemy(ChessPieceType lhs, ChessPieceType rhs);
    bool is_red(ChessPieceType piece_type);
    bool is_red(const Pos& pos);
    bool is_black(ChessPieceType piece_type);
    bool is_black(const Pos& pos);
    bool is_empty(ChessPieceType piece_type);
    bool is_empty(const Pos& pos);
    bool in_red_area(const Pos & pos);
    bool in_black_area(const Pos& pos);

    ChessPieceType get_piece_type(const Pos& pos);
    Pos selected_pos { -1, -1 };
    static inline const Pos NONE_POS { -1, -1 };
    void load_image();
    void load_image_data();
    void render();
    void render_chess_pieces();
    void render_chess_board();
    void process_click(int x_pos, int y_pos);
    HDC g_hdc { nullptr }, g_mdc { nullptr }, g_bufdc { nullptr };
    static constexpr int chess_piece_height { 100 }, chess_piece_width { 100 };
    static constexpr int board_height { 10 }, board_width { 9 };
    static constexpr int board_bmp_width { 900 }, board_bmp_height { 1000 };
    static bool find_in_board(const std::vector<std::vector<ChessPieceType>>& chess_board, const std::vector<Pos>& pos, ChessPieceType target_type);

    void render_empty_chess();

    std::vector<Pos> black_soldier_move(int width_idx, int height_idx);
	std::vector<Pos> black_elephant_move(int width_idx, int height_idx);
	std::vector<Pos> black_guard_move(int width_idx, int height_idx);
	std::vector<Pos> black_king_move(int width_idx, int height_idx);

    std::vector<Pos> red_soldier_move(int width_idx, int height_idx);
	std::vector<Pos> red_elephant_move(int width_idx, int height_idx);
	std::vector<Pos> red_guard_move(int width_idx, int height_idx);
	std::vector<Pos> red_king_move(int width_idx, int height_idx);

	std::vector<Pos> cannon_move(int width_idx, int height_idx, ChessPieceType cannon_type);
	std::vector<Pos> rook_move(int width_idx, int height_idx, ChessPieceType rook_type);
	std::vector<Pos> knight_move(int width_idx, int height_idx, ChessPieceType knight_type);
	std::vector<Pos> elephant_move(int width_idx, int height_idx, ChessPieceType elephant_type);
	std::vector<Pos> guard_move(int width_idx, int height_idx, ChessPieceType guard_type);

    bool black_win();
    bool red_win();
    void game_end_dlg(bool is_red_win);
    static bool pos_in_board(Pos pos);
    static bool pos_in_board(int width_idx, int height_idx);
    static PosOffset get_knight_foot(PosOffset pos);
    static INT_PTR CALLBACK game_end_dlg_proc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static constexpr Pos red_king_left_top_pos{ 3,7 };
	static constexpr Pos red_king_right_bottom_pos(5, 9);
};
