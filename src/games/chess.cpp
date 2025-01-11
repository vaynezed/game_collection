#include "games/chess.hpp"
#include "../dlg.hpp"
#include "games/utils.hpp"
#include <windowsx.h> // Correct the typo in the include directive

void Chess::game_init(HWND& hwnd)
{
    this->main_hwnd_ptr = &hwnd;
    this->g_hdc = GetDC(hwnd);
    this->g_mdc = CreateCompatibleDC(this->g_hdc);
    this->g_bufdc = CreateCompatibleDC(g_hdc);
    HBITMAP bmp = CreateCompatibleBitmap(g_hdc, screen_width, screen_height);
    SelectObject(g_mdc, bmp);
    DeleteObject(bmp);
    this->load_image();
    this->load_image_data();
    this->game_init_flag = true;
    this->is_red_player = true;
}

void Chess::game_loop()
{
    g_tnow = GetTickCount64();
    bool need_re_rend { g_tnow - g_tpre >= 50 };
    if (need_re_rend) {
        g_tpre = GetTickCount64();
        this->render();
        this->check_game_data();
    }
}

void Chess::check_game_data()
{
    bool black_win_flag = this->black_win();
    bool red_win_flag = this->red_win();
    if (black_win_flag) {
        this->game_end_dlg(false);
    } else if (red_win_flag) {
        this->game_end_dlg(true);
    }
}

void Chess::game_cleanup()
{
    // Release BMPS
    DeleteObject(this->chess_bmp);
    DeleteObject(this->black_cannon);
    DeleteObject(this->black_elephant);
    DeleteObject(this->black_guard);
    DeleteObject(this->black_king);
    DeleteObject(this->black_knight);
    DeleteObject(this->black_rook);
    DeleteObject(this->black_soldier);
    DeleteObject(this->red_cannon);
    DeleteObject(this->red_elephant);
    DeleteObject(this->red_guard);
    DeleteObject(this->red_king);
    DeleteObject(this->red_knight);
    DeleteObject(this->red_rook);
    DeleteObject(this->red_soldier);

    // Release DCs
    DeleteDC(this->g_mdc);
    DeleteDC(this->g_bufdc);
    ReleaseDC(*main_hwnd_ptr, g_hdc);

    this->chess_board.clear();
    this->selected_pos = NONE_POS;
    this->g_tnow = this->g_tpre = 0;
    this->chess_target_pos_s.clear();

    main_hwnd_ptr = nullptr;
    this->game_init_flag = false;
}

void Chess::game_process_key_down(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_KEYDOWN: {
        switch (wParam) {
        case VK_ESCAPE: {
            this->show_dlg(ID_RESTART_DLG);
            break;
        }
        }
        break;
    }
    case WM_LBUTTONDOWN: {

        int width_pos = GET_X_LPARAM(lParam);
        int height_pos = GET_Y_LPARAM(lParam);
        this->process_click(width_pos, height_pos);
        break;
    }
    default:
        break;
    }
}

std::wstring Chess::to_string()
{
    return std::wstring(L"象棋");
}

HBITMAP* Chess::get_bmp_ptr_from_pos_type(ChessPieceType pos_type)
{
    HBITMAP* ret = nullptr;
    if (pos_type == ChessPieceType::black_cannon)
        ret = &this->black_cannon;
    else if (pos_type == ChessPieceType::black_elephant)
        ret = &this->black_elephant;
    else if (pos_type == ChessPieceType::black_guard)
        ret = &this->black_guard;
    else if (pos_type == ChessPieceType::black_king)
        ret = &this->black_king;
    else if (pos_type == ChessPieceType::black_knight)
        ret = &this->black_knight;
    else if (pos_type == ChessPieceType::black_rook)
        ret = &this->black_rook;
    else if (pos_type == ChessPieceType::black_soldier)
        ret = &this->black_soldier;
    else if (pos_type == ChessPieceType::red_cannon)
        ret = &this->red_cannon;
    else if (pos_type == ChessPieceType::red_elephant)
        ret = &this->red_elephant;
    else if (pos_type == ChessPieceType::red_guard)
        ret = &this->red_guard;
    else if (pos_type == ChessPieceType::red_king)
        ret = &this->red_king;
    else if (pos_type == ChessPieceType::red_knight)
        ret = &this->red_knight;
    else if (pos_type == ChessPieceType::red_rook)
        ret = &this->red_rook;
    else if (pos_type == ChessPieceType::red_soldier)
        ret = &this->red_soldier;
    return ret;
}

inline bool Chess::is_enemy(ChessPieceType lhs, ChessPieceType rhs)
{
    bool ret = false;
    if (this->is_black(lhs) && this->is_red(rhs)) {
        ret = true;
    }
    else if (this->is_red(lhs) && this->is_black(rhs)) {
        ret = true;
    }
    return ret;
}

inline bool Chess::is_red(ChessPieceType piece_type)
{
    int piece_id = static_cast<int>(piece_type);
    bool ret = piece_id > 10 && piece_id < 20;
    return ret;
}

inline bool Chess::is_red(const Pos& pos)
{
    ChessPieceType chess_piece_type = this->get_piece_type(pos);
    bool ret = this->is_red(chess_piece_type);
    return ret;
}

inline bool Chess::is_black(ChessPieceType piece_type)
{
    int piece_id = static_cast<int>(piece_type);
    bool ret = piece_id > 0 && piece_id < 10;
    return ret;
}

inline bool Chess::is_black(const Pos& pos)
{
    ChessPieceType chess_piece_type = this->get_piece_type(pos);
    bool ret = this->is_black(chess_piece_type);
    return ret;
}

inline bool Chess::is_empty(ChessPieceType piece_type)
{
    int piece_id = static_cast<int>(piece_type);
    bool ret = piece_id == 0;
    return ret;
}

inline bool Chess::is_empty(const Pos& pos)
{
    ChessPieceType chess_piece_type = this->get_piece_type(pos);
    bool ret = this->is_empty(chess_piece_type);
    return ret;
}

bool Chess::in_red_area(const Pos& pos)
{
    int h_pos = pos.h_pos;
    bool ret = false;
    if (h_pos >= 5 && h_pos <= 9) {
        ret = true;
    }
    return ret;
}

bool Chess::in_black_area(const Pos& pos)
{
    int h_pos = pos.h_pos;
    bool ret = false;
    if (h_pos >= 0 && h_pos <= 4) {
        ret = true;
    }
    return ret;
}

Chess::ChessPieceType Chess::get_piece_type(const Pos& pos)
{
    return this->chess_board[pos.h_pos][pos.w_pos];
}

void Chess::load_image()
{
    this->chess_bmp = (HBITMAP)LoadImage(NULL, TEXT("./resource/chess/chess_board.bmp"), IMAGE_BITMAP, board_bmp_width, board_bmp_height, LR_LOADFROMFILE);

    this->empty_chess = (HBITMAP)LoadImage(NULL, TEXT("./resource/chess/empty_chess.bmp"), IMAGE_BITMAP, chess_piece_width, chess_piece_height, LR_LOADFROMFILE);

    this->black_cannon = (HBITMAP)LoadImage(NULL, TEXT("./resource/chess/black_cannon.bmp"), IMAGE_BITMAP, 100, 100, LR_LOADFROMFILE);
    this->black_elephant = (HBITMAP)LoadImage(NULL, TEXT("./resource/chess/black_elephant.bmp"), IMAGE_BITMAP, 100, 100, LR_LOADFROMFILE);
    this->black_guard = (HBITMAP)LoadImage(NULL, TEXT("./resource/chess/black_guard.bmp"), IMAGE_BITMAP, 100, 100, LR_LOADFROMFILE);
    this->black_king = (HBITMAP)LoadImage(NULL, TEXT("./resource/chess/black_king.bmp"), IMAGE_BITMAP, 100, 100, LR_LOADFROMFILE);
    this->black_knight = (HBITMAP)LoadImage(NULL, TEXT("./resource/chess/black_knight.bmp"), IMAGE_BITMAP, 100, 100, LR_LOADFROMFILE);
    this->black_rook = (HBITMAP)LoadImage(NULL, TEXT("./resource/chess/black_rook.bmp"), IMAGE_BITMAP, 100, 100, LR_LOADFROMFILE);
    this->black_soldier = (HBITMAP)LoadImage(NULL, TEXT("./resource/chess/black_soldier.bmp"), IMAGE_BITMAP, 100, 100, LR_LOADFROMFILE);

    this->red_cannon = (HBITMAP)LoadImage(NULL, TEXT("./resource/chess/red_cannon.bmp"), IMAGE_BITMAP, 100, 100, LR_LOADFROMFILE);
    this->red_elephant = (HBITMAP)LoadImage(NULL, TEXT("./resource/chess/red_elephant.bmp"), IMAGE_BITMAP, 100, 100, LR_LOADFROMFILE);
    this->red_guard = (HBITMAP)LoadImage(NULL, TEXT("./resource/chess/red_guard.bmp"), IMAGE_BITMAP, 100, 100, LR_LOADFROMFILE);
    this->red_king = (HBITMAP)LoadImage(NULL, TEXT("./resource/chess/red_king.bmp"), IMAGE_BITMAP, 100, 100, LR_LOADFROMFILE);
    this->red_knight = (HBITMAP)LoadImage(NULL, TEXT("./resource/chess/red_knight.bmp"), IMAGE_BITMAP, 100, 100, LR_LOADFROMFILE);
    this->red_rook = (HBITMAP)LoadImage(NULL, TEXT("./resource/chess/red_rook.bmp"), IMAGE_BITMAP, 100, 100, LR_LOADFROMFILE);
    this->red_soldier = (HBITMAP)LoadImage(NULL, TEXT("./resource/chess/red_soldier.bmp"), IMAGE_BITMAP, 100, 100, LR_LOADFROMFILE);
}

void Chess::load_image_data()
{
    this->chess_board.resize(board_height, std::vector<ChessPieceType>(board_width, ChessPieceType::empty));
    this->chess_board[0][0] = ChessPieceType::black_rook;
    this->chess_board[0][1] = ChessPieceType::black_knight;
    this->chess_board[0][2] = ChessPieceType::black_elephant;
    this->chess_board[0][3] = ChessPieceType::black_guard;
    this->chess_board[0][4] = ChessPieceType::black_king;
    this->chess_board[0][5] = ChessPieceType::black_guard;
    this->chess_board[0][6] = ChessPieceType::black_elephant;
    this->chess_board[0][7] = ChessPieceType::black_knight;
    this->chess_board[0][8] = ChessPieceType::black_rook;

    this->chess_board[2][1] = ChessPieceType::black_cannon;
    this->chess_board[2][7] = ChessPieceType::black_cannon;

    this->chess_board[3][0] = ChessPieceType::black_soldier;
    this->chess_board[3][2] = ChessPieceType::black_soldier;
    this->chess_board[3][4] = ChessPieceType::black_soldier;
    this->chess_board[3][6] = ChessPieceType::black_soldier;
    this->chess_board[3][8] = ChessPieceType::black_soldier;

    this->chess_board[6][0] = ChessPieceType::red_soldier;
    this->chess_board[6][2] = ChessPieceType::red_soldier;
    this->chess_board[6][4] = ChessPieceType::red_soldier;
    this->chess_board[6][6] = ChessPieceType::red_soldier;
    this->chess_board[6][8] = ChessPieceType::red_soldier;

    this->chess_board[7][1] = ChessPieceType::red_cannon;
    this->chess_board[7][7] = ChessPieceType::red_cannon;

    this->chess_board[9][0] = ChessPieceType::red_rook;
    this->chess_board[9][1] = ChessPieceType::red_knight;
    this->chess_board[9][2] = ChessPieceType::red_elephant;
    this->chess_board[9][3] = ChessPieceType::red_guard;
    this->chess_board[9][4] = ChessPieceType::red_king;
    this->chess_board[9][5] = ChessPieceType::red_guard;
    this->chess_board[9][6] = ChessPieceType::red_elephant;
    this->chess_board[9][7] = ChessPieceType::red_knight;
    this->chess_board[9][8] = ChessPieceType::red_rook;
}

void Chess::render()
{
    game::util::draw_blackground(&this->g_mdc, screen_width, screen_height, WHITE_BRUSH);

    this->render_chess_board();
    this->render_chess_pieces();
    this->render_empty_chess();
    BitBlt(this->g_hdc, screen_width / 2 - board_bmp_width / 2, screen_height / 2 - board_bmp_height / 2, screen_width, screen_height, this->g_mdc, 0, 0, SRCCOPY);
}

void Chess::render_chess_pieces()
{
    for (int h_idx = 0; h_idx < 10; ++h_idx) {
        for (int w_idx = 0; w_idx < 9; ++w_idx) {
            ChessPieceType pos_type = this->chess_board[h_idx][w_idx];
            HBITMAP* bmp_ptr = this->get_bmp_ptr_from_pos_type(pos_type);
            if (bmp_ptr != nullptr) {
                SelectObject(g_bufdc, *bmp_ptr);
                TransparentBlt(g_mdc, w_idx * chess_piece_width, h_idx * chess_piece_height, chess_piece_width, chess_piece_height, g_bufdc, 0, 0, chess_piece_width, chess_piece_height, RGB(255, 255, 255));
            }
        }
    }
}

void Chess::render_chess_board()
{
    // Draw the chess board
    SelectObject(g_bufdc, this->chess_bmp);
    BitBlt(this->g_mdc, 0, 0, board_bmp_width, board_bmp_height, this->g_bufdc, 0, 0, SRCCOPY);
}

void Chess::process_click(int width_pos, int height_pos)
{
    int offset_x_from_chess = width_pos - (screen_width / 2 - board_bmp_width / 2);
    int offset_y_from_chess = height_pos - (screen_height / 2 - board_bmp_height / 2);
    int chess_piece_height_idx = offset_y_from_chess / chess_piece_height;
    int chess_piece_width_idx = offset_x_from_chess / chess_piece_width;
    bool pos_in_board = chess_piece_width_idx >= 0 && chess_piece_width_idx < board_width && chess_piece_height_idx >= 0 && chess_piece_height_idx < board_height;
    if (pos_in_board) {

        Pos current_pos { chess_piece_width_idx, chess_piece_height_idx };
        ChessPieceType chess_piece_type = this->chess_board[chess_piece_height_idx][chess_piece_width_idx];
        bool has_selected_chess_piece = this->selected_pos != NONE_POS;
        if (has_selected_chess_piece == false) {
            this->selected_pos = current_pos;
            if (this->is_red_player == false) {
                if (chess_piece_type == ChessPieceType::black_soldier) {
                    this->chess_target_pos_s = std::move(black_soldier_move(chess_piece_width_idx, chess_piece_height_idx));
                }
				else if (chess_piece_type == ChessPieceType::black_cannon) {
					this->chess_target_pos_s = std::move(this->cannon_move(chess_piece_width_idx, chess_piece_height_idx, ChessPieceType::black_cannon));
				}
				else if (chess_piece_type == ChessPieceType::black_rook) {
					this->chess_target_pos_s = std::move(this->rook_move(chess_piece_width_idx, chess_piece_height_idx, ChessPieceType::black_rook));
				}
				else if (chess_piece_type == ChessPieceType::black_knight) {
					this->chess_target_pos_s = std::move(this->knight_move(chess_piece_width_idx, chess_piece_height_idx, ChessPieceType::black_knight));
				}
                else if (chess_piece_type == ChessPieceType::black_elephant) {
					this->chess_target_pos_s = std::move(this->elephant_move(chess_piece_width_idx, chess_piece_height_idx, ChessPieceType::black_elephant));
                }
            } else {
                if (chess_piece_type == ChessPieceType::red_soldier) {
                    this->chess_target_pos_s = std::move(this->red_soldier_move(chess_piece_width_idx, chess_piece_height_idx));
                }
				else if (chess_piece_type == ChessPieceType::red_cannon) {
					this->chess_target_pos_s = std::move(this->cannon_move(chess_piece_width_idx, chess_piece_height_idx, ChessPieceType::red_cannon));
				}
				else if (chess_piece_type == ChessPieceType::red_rook) {
					this->chess_target_pos_s = std::move(this->rook_move(chess_piece_width_idx, chess_piece_height_idx, ChessPieceType::red_rook));
				}
				else if (chess_piece_type == ChessPieceType::red_knight) {
					this->chess_target_pos_s = std::move(this->knight_move(chess_piece_width_idx, chess_piece_height_idx, ChessPieceType::red_knight));
				}
                else if (chess_piece_type == ChessPieceType::red_elephant) {
					this->chess_target_pos_s = std::move(this->elephant_move(chess_piece_width_idx, chess_piece_height_idx, ChessPieceType::red_elephant));
                }
            }
        } else {
            bool in_target_pos = find(this->chess_target_pos_s.begin(), this->chess_target_pos_s.end(), current_pos) != this->chess_target_pos_s.end();
            if (in_target_pos) {
                ChessPieceType pre_chess_piece_type = this->chess_board[this->selected_pos.h_pos][this->selected_pos.w_pos];
                this->chess_board[current_pos.h_pos][current_pos.w_pos] = pre_chess_piece_type;
                this->chess_board[this->selected_pos.h_pos][this->selected_pos.w_pos] = ChessPieceType::empty;
                this->is_red_player = !this->is_red_player;
            }
            this->chess_target_pos_s.clear();
            this->selected_pos = NONE_POS;
        }
    }
}

std::vector<Pos> Chess::black_soldier_move(int width_idx, int height_idx)
{
    std::vector<Pos> pos_s;
    if (height_idx < 5) {
        Pos pos { width_idx, height_idx + 1 };
        pos_s.push_back(pos);
    } else {
        Pos pos { width_idx, height_idx };

        PosOffset offsets[] = {
            { 1, 0 },
            { -1, 0 },
            { 0, 1 },
        };
        int offset_len = array_length(offsets);

        for (int idx = 0; idx < offset_len; ++idx) {
            Pos target_pos = pos + offsets[idx];
            if (target_pos.h_pos >= 0 && target_pos.h_pos < board_height && target_pos.w_pos >= 0 && target_pos.w_pos < board_width) {
                ChessPieceType target_pos_type = this->chess_board[target_pos.h_pos][target_pos.w_pos];
                ChessPieceType current_pos_type = this->chess_board[height_idx][width_idx];
                if (target_pos_type == ChessPieceType::empty) {
                    pos_s.push_back(target_pos);
                } else if (is_enemy(current_pos_type, target_pos_type)) {
                    pos_s.push_back(target_pos);
                }
            }
        }
    }
    return pos_s;
}

std::vector<Pos> Chess::black_elephant_move(int width_idx, int height_idx)
{
    return std::vector<Pos>();
}

std::vector<Pos> Chess::black_guard_move(int width_idx, int height_idx)
{
    return std::vector<Pos>();
}

std::vector<Pos> Chess::black_king_move(int width_idx, int height_idx)
{
    return std::vector<Pos>();
}

std::vector<Pos> Chess::red_soldier_move(int width_idx, int height_idx)
{
    std::vector<Pos> pos_s;
    if (height_idx >= 5) {
        Pos pos { width_idx, height_idx - 1 };
        pos_s.push_back(pos);
    } else {
        Pos pos { width_idx, height_idx };

        PosOffset offsets[] = {
            { 1, 0 },
            { -1, 0 },
            { 0, -1 },
        };
        int offset_len = array_length(offsets);

        for (int idx = 0; idx < offset_len; ++idx) {
            Pos target_pos = pos + offsets[idx];
            if (target_pos.h_pos >= 0 && target_pos.h_pos < board_height && target_pos.w_pos >= 0 && target_pos.w_pos < board_width) {
                ChessPieceType target_pos_type = this->chess_board[target_pos.h_pos][target_pos.w_pos];
                ChessPieceType current_pos_type = this->chess_board[height_idx][width_idx];
                if (target_pos_type == ChessPieceType::empty) {
                    pos_s.push_back(target_pos);
                } else if (is_enemy(current_pos_type, target_pos_type)) {
                    pos_s.push_back(target_pos);
                }
            }
        }
    }
    return pos_s;
}




std::vector<Pos> Chess::red_elephant_move(int width_idx, int height_idx)
{
    return std::vector<Pos>();
}

std::vector<Pos> Chess::red_guard_move(int width_idx, int height_idx)
{
    return std::vector<Pos>();
}

std::vector<Pos> Chess::red_king_move(int width_idx, int height_idx)
{
    return std::vector<Pos>();
}

std::vector<Pos> Chess::cannon_move(int width_idx, int height_idx, ChessPieceType cannon_type)
{
	std::vector<Pos> ret;
	PosOffset offsets[] = {
		{ 1, 0 },
		{ -1, 0 },
		{ 0, 1 },
		{ 0, -1 },
	};
	int offsets_len = array_length(offsets);
    for (int idx = 0;idx < offsets_len;++idx) {
		PosOffset& offset = offsets[idx];
		Pos source_pos{ width_idx, height_idx };
        while (true) {
			source_pos += offset;
            bool pos_in_board_flag = Chess::pos_in_board(source_pos);
			if (pos_in_board_flag == false) {
				break;
			}

			ChessPieceType type = this->chess_board[source_pos.h_pos][source_pos.w_pos];
            if (type == ChessPieceType::empty) {
				ret.push_back(source_pos);
            }
            else{
                while (true) {
					source_pos += offset;
                    bool pos_in_board_flag = Chess::pos_in_board(source_pos);
					if (pos_in_board_flag == false) {
						break;
					}
					ChessPieceType type = this->chess_board[source_pos.h_pos][source_pos.w_pos];

                    if (this->is_enemy(type, cannon_type)) {
						ret.push_back(source_pos);
                        break;
                    }
                }
                break;
			}
        }
    }
    return ret;
}

std::vector<Pos> Chess::rook_move(int width_idx, int height_idx, ChessPieceType rook_type)
{
	std::vector<Pos> ret;
	PosOffset offsets[] = {
		{ 1, 0 },
		{ -1, 0 },
		{ 0, 1 },
		{ 0, -1 },
	};
	int offsets_len = array_length(offsets);
    for (int idx = 0;idx < offsets_len;++idx) {
		PosOffset& offset = offsets[idx];
		Pos source_pos{ width_idx, height_idx };
        while (true) {
			source_pos += offset;
			if (source_pos.h_pos < 0 || source_pos.h_pos >= board_height || source_pos.w_pos < 0 || source_pos.w_pos >= board_width) {
				break;
			}

			ChessPieceType type = this->chess_board[source_pos.h_pos][source_pos.w_pos];
            if (type == ChessPieceType::empty) {
				ret.push_back(source_pos);
            }
            else {
				bool is_enemy_flag = this->is_enemy(type, rook_type);
                if (is_enemy_flag) {
                    ret.push_back(source_pos);
                }
                break;
            }
        }
    }
    return ret;
}

std::vector<Pos> Chess::knight_move(int width_idx, int height_idx, ChessPieceType knight_type)
{
    std::vector<Pos> ret;
	PosOffset offsets[] = {
		{ 2, 1 },
		{ 2, -1 },
		{ -2, 1 },
		{ -2, -1 },
        {1,2},
        {1,-2},
        {-1,2},
        {-1,-2}
	};
    Pos source_pos(width_idx, height_idx);
    int offsets_len = array_length(offsets);
    for (int idx = 0;idx < offsets_len;++idx) {
        PosOffset& offset = offsets[idx];
        PosOffset foot_offset = this->get_knight_foot(offset);
		Pos foot_pos = source_pos + foot_offset;
		bool pos_in_board = Chess::pos_in_board(foot_pos);

		if (pos_in_board && this->is_empty(foot_pos)) {
			Pos target_pos = source_pos + offset;
			bool target_pos_in_board = Chess::pos_in_board(target_pos);
            if (target_pos_in_board) {
                ChessPieceType piece_type = this->get_piece_type(target_pos);
                bool is_enemy_flag = this->is_enemy(knight_type, piece_type);
                bool is_empty_flag = this->is_empty(target_pos);
				if (is_empty_flag || is_enemy_flag) {
					ret.push_back(target_pos);
				}
            }
		}
    }
    return ret;
}

std::vector<Pos> Chess::elephant_move(int width_idx, int height_idx, ChessPieceType elephant_type)
{
    PosOffset offsets[] = {
        {2,2},
        {2,-2},
        {-2,2},
        {-2,-2}
    };
    int  offsets_len = array_length(offsets);
	Pos source_pos(width_idx, height_idx);
    std::vector<Pos> ret;
    for (int idx = 0;idx < offsets_len;idx++) {
		PosOffset& offset = offsets[idx];
		Pos target_pos = source_pos + offset;
        if (this->pos_in_board(target_pos)) {
			if (  is_red(elephant_type) && this->in_red_area(target_pos)) {
				ret.push_back(target_pos);
			}
            else if (is_black(elephant_type) && this->in_black_area(target_pos)) {
                ret.push_back(target_pos);
            }
        }
    }
    return ret;
}

std::vector<Pos> Chess::guard_move(int width_idx, int height_idx, ChessPieceType guard_type)
{
    std::vector<Pos> ret;
    PosOffset offsets[] = {
        {2,2},
        {2,-2},
        {-2,2},
        {-2,-2}
    };
    return ret;
}

bool Chess::black_win()
{
    static const std::vector<Pos> red_king_pos_s = {
        { 3, 7 },
        { 4, 7 },
        { 5, 7 },
        { 3, 8 },
        { 4, 8 },
        { 5, 8 },
        { 3, 9 },
        { 4, 9 },
        { 5, 9 },
    };
    bool ret = !this->find_in_board(this->chess_board, red_king_pos_s, ChessPieceType::red_king);
    return ret;
}

bool Chess::red_win()
{
    static const std::vector<Pos> black_king_pos_s = {
        { 3, 0 },
        { 4, 0 },
        { 5, 0 },
        { 3, 1 },
        { 4, 1 },
        { 5, 1 },
        { 3, 2 },
        { 4, 2 },
        { 5, 2 },
    };
    bool ret = !this->find_in_board(this->chess_board, black_king_pos_s, ChessPieceType::black_king);
    return ret;
}

void Chess::game_end_dlg(bool is_red_win)
{
    HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(*main_hwnd_ptr, GWLP_HINSTANCE);
    int ret = DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_VICTORY),
        *main_hwnd_ptr, Chess::game_end_dlg_proc, (LPARAM)&is_red_win);
    if (ret == ID_GOBACK) {
        goback_main_menu();
    } else if (ret == ID_RESTART_BTN) {
        HWND* main_hwnd_ptr = this->main_hwnd_ptr;
        game_cleanup();
        game_init(*main_hwnd_ptr);
    }
}

inline bool Chess::pos_in_board(Pos pos)
{
    return  Chess::pos_in_board(pos.w_pos, pos.h_pos);
}

inline bool Chess::pos_in_board(int width_idx, int height_idx)
{
    bool ret = (width_idx >= 0 && width_idx < Chess::board_width && height_idx >= 0 && height_idx < Chess::board_height);
    return ret;
}

PosOffset Chess::get_knight_foot(PosOffset pos)
{
    int width_offset = pos.width_offset, height_offset = pos.height_offset;
    PosOffset pos_offset{ -1,-1 };
    if (abs(width_offset) == 2) {
        pos_offset = PosOffset(width_offset / 2, 0);
    }
    else if (abs(height_offset) == 2) {
        pos_offset = PosOffset(0, height_offset / 2);
    }
    return pos_offset;
}

INT_PTR Chess::game_end_dlg_proc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_INITDIALOG: {
        HWND parent_hwnd = GetParent(hDlg);
        EnableWindow(parent_hwnd, FALSE);
        CenterDialog(hDlg);
        bool* is_red_win = reinterpret_cast<bool*>(lParam);
        const TCHAR* red_win_str = TEXT("红方胜利");
        const TCHAR* black_win_str = TEXT("黑方胜利");
        const TCHAR* victory_static_str = is_red_win ? red_win_str : black_win_str;
        SetDlgItemText(hDlg, IDC_VICTORY_STATIC, victory_static_str);
        return TRUE;
    }
    case WM_COMMAND: {
        INT32 btn_id = LOWORD(wParam);
        EndDialog(hDlg, btn_id);
        return TRUE;
        break;
    }
    case WM_DESTROY: {
        HWND parent_hwnd = GetParent(hDlg);
        EnableWindow(parent_hwnd, TRUE);
        break;
    }

    default:
        return FALSE;
    }
    return FALSE;
}

bool Chess::find_in_board(const std::vector<std::vector<ChessPieceType>>& chess_board, const std::vector<Pos>& pos, ChessPieceType target_type)
{
    int size = static_cast<int>(pos.size());
    bool ret = false;
    for (int idx = 0; idx < size; ++idx) {
        const Pos& p = pos[idx];
        bool pos_in_board = Chess::pos_in_board(p);
        if (pos_in_board) {
            ChessPieceType target_pos_type = chess_board[p.h_pos][p.w_pos];
            if (target_pos_type == target_type) {
                ret = true;
                break;
            }
        }
    }
    return ret;
}

void Chess::render_empty_chess()
{
    std::vector<Pos>& pos_s = this->chess_target_pos_s;
    for (const Pos& pos : pos_s) {
        const int w_idx = pos.w_pos, h_idx = pos.h_pos;
        SelectObject(g_bufdc, this->empty_chess);
        TransparentBlt(g_mdc, w_idx * chess_piece_width, h_idx * chess_piece_height, chess_piece_width, chess_piece_height, g_bufdc, 0, 0, chess_piece_width, chess_piece_height, RGB(255, 255, 255));
    }
    BitBlt(this->g_hdc, screen_width / 2 - board_bmp_width / 2, screen_height / 2 - board_bmp_height / 2, screen_width, screen_height, this->g_mdc, 0, 0, SRCCOPY);
}
