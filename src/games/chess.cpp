#include "../resource.h"
#include <algorithm>
#include <windowsx.h>

#include "dlg.hpp"
#include "games/chess.hpp"
#include "games/utils.hpp"
#include "nlohmann/json.hpp"

const Pos Chess::red_king_left_top_pos { 3, 7 };
const Pos Chess::red_king_right_bottom_pos { 5, 9 };
const Pos Chess::black_king_left_top_pos { 3, 0 };
const Pos Chess::black_king_right_bottom_pos { 5, 2 };
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
    this->load_system_resource();
    this->game_init_flag = true;
    this->red_turn = true;
    this->init_opponent();
}
const std::string Chess::localhost = "127.0.0.1";

void Chess::init_opponent()
{
    if (this->game_model_idx == Chess::AI_MODEL) {
        this->opponent = std::make_unique<AIOpponent>(this, true);
    } else if (this->game_model_idx == Chess::NET_MODEL) {
        HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(*main_hwnd_ptr, GWLP_HINSTANCE);
        LoginDialog login_dlg;

        Server* server = reinterpret_cast<Server*>(get_game_server(localhost, port));
        this->server.reset(server);

        int code = 1;
        do {
            login_dlg.show(this->main_hwnd_ptr);
            struct user_info_t user_info = login_dlg.get_data();
            int room_id = std::stoi(user_info.room_id);
            this->opponent = std::make_unique<WebOpponent>(this, room_id, this->server);
            WebOpponent* web_opponent = reinterpret_cast<WebOpponent*>(this->opponent.get());

            WebOpponent::EnterRoomRsp rsp = web_opponent->enter_room();
            code = rsp.code;
            if (code == 0) {
                MessageBoxA(*main_hwnd_ptr, rsp.message.c_str(), "进入房间失败", MB_OK);
            }
        } while (code == 0);
    } else if (this->game_model_idx == Chess::PAIR_MODEL) {
        this->opponent = std::make_unique<PairOpponent>(this);
    }
}

void Chess::game_loop()
{
    g_tnow = GetTickCount64();
    bool need_re_rend { g_tnow - g_tpre >= 50 };
    if (need_re_rend) {
        g_tpre = GetTickCount64();
        this->render();
        this->opponent_check_move();
        this->check_game_data();
    }
}

void Chess::opponent_check_move()
{
    this->opponent.get()->move();
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

    SelectObject(this->g_mdc, hfont_old);
    DeleteObject(this->hfont_new);
    // Release DCs
    DeleteDC(this->g_mdc);
    DeleteDC(this->g_bufdc);
    ReleaseDC(*main_hwnd_ptr, g_hdc);

    this->chess_board.clear();
    this->selected_pos = NONE_POS;
    this->g_tnow = this->g_tpre = 0;
    this->chess_target_pos_s.clear();

    if (this->game_model_idx == Chess::NET_MODEL) {
        WebOpponent* web_opponent = reinterpret_cast<WebOpponent*>(this->opponent.get());
        web_opponent->leave_room();
        this->server->close_connection();
    }

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

std::vector<std::wstring> Chess::game_models()
{
    std::vector<std::wstring> ret { { L"AI",
        L"双人",
        L"联机" } };
    return ret;
}

void Chess::set_game_model(int idx)
{
    this->game_model_idx = idx;
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
    } else if (this->is_red(lhs) && this->is_black(rhs)) {
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

inline bool Chess::in_red_area(const Pos& pos)
{
    int h_pos = pos.h_pos;
    bool ret = false;
    if (this->pos_in_board(pos) && h_pos >= 5 && h_pos <= 9) {
        ret = true;
    }
    return ret;
}

inline bool Chess::in_black_area(const Pos& pos)
{
    int h_pos = pos.h_pos;
    bool ret = false;
    if (this->pos_in_board(pos) && h_pos >= 0 && h_pos <= 4) {
        ret = true;
    }
    return ret;
}

inline bool Chess::in_red_king_area(const Pos& pos)
{
    return pos.in_area(Chess::red_king_left_top_pos, Chess::red_king_right_bottom_pos);
}

inline bool Chess::in_black_king_area(const Pos& pos)
{
    return pos.in_area(Chess::black_king_left_top_pos, Chess::black_king_right_bottom_pos);
}

inline bool Chess::is_red_turn()
{
    return this->red_turn;
}

void Chess::next_turn()
{
    this->red_turn = !this->red_turn;
}

inline const Chess::ChessPieceType Chess::get_piece_type(const Pos& pos)
{
    return this->chess_board[pos.h_pos][pos.w_pos];
}

inline void Chess::set_piece_type(const Pos& pos, ChessPieceType piece_type)
{
    this->chess_board[pos.h_pos][pos.w_pos] = piece_type;
}

inline void Chess::load_image()
{
    HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(*main_hwnd_ptr, GWLP_HINSTANCE);
    this->chess_bmp = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_CHESS_BOARD), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);

    this->empty_chess = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_EMPTY_CHESS), IMAGE_BITMAP, chess_piece_width, chess_piece_height, LR_CREATEDIBSECTION);

    this->black_cannon = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_BLACK_CANNON), IMAGE_BITMAP, 100, 100, LR_CREATEDIBSECTION);
    this->black_elephant = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_BLACK_ELEPHANT), IMAGE_BITMAP, 100, 100, LR_CREATEDIBSECTION);
    this->black_guard = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_BLACK_GUARD), IMAGE_BITMAP, 100, 100, LR_CREATEDIBSECTION);
    this->black_king = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_BLACK_KING), IMAGE_BITMAP, 100, 100, LR_CREATEDIBSECTION);
    this->black_knight = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_BLACK_KNIGHT), IMAGE_BITMAP, 100, 100, LR_CREATEDIBSECTION);
    this->black_rook = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_BLACK_ROOK), IMAGE_BITMAP, 100, 100, LR_CREATEDIBSECTION);
    this->black_soldier = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_BLACK_SOLDIER), IMAGE_BITMAP, 100, 100, LR_CREATEDIBSECTION);

    this->red_cannon = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_RED_CANNON), IMAGE_BITMAP, 100, 100, LR_CREATEDIBSECTION);
    this->red_elephant = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_RED_ELEPHANT), IMAGE_BITMAP, 100, 100, LR_CREATEDIBSECTION);
    this->red_guard = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_RED_GUARD), IMAGE_BITMAP, 100, 100, LR_CREATEDIBSECTION);
    this->red_king = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_RED_KING), IMAGE_BITMAP, 100, 100, LR_CREATEDIBSECTION);
    this->red_knight = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_RED_KNIGHT), IMAGE_BITMAP, 100, 100, LR_CREATEDIBSECTION);
    this->red_rook = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_RED_ROOK), IMAGE_BITMAP, 100, 100, LR_CREATEDIBSECTION);
    this->red_soldier = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_RED_SOLDIER), IMAGE_BITMAP, 100, 100, LR_CREATEDIBSECTION);
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

void Chess::load_system_resource()
{
    hfont_new = CreateFont(
        60, // 字体高度
        0, // 字体宽度
        0, // 字符串转角（角度）
        0, // 字符倾斜度
        FW_NORMAL, // 字体粗细
        FALSE, // 是否斜体
        FALSE, // 是否下划线
        FALSE, // 是否删除线
        ANSI_CHARSET, // 字符集
        OUT_DEFAULT_PRECIS, // 输出精度
        CLIP_DEFAULT_PRECIS, // 字符串剪切精度
        DEFAULT_QUALITY, // 输出质量
        DEFAULT_PITCH | FF_SWISS, // 字符间距和字体族
        TEXT("Arial") // 字体名称
    );
    hfont_old = (HFONT)SelectObject(this->g_mdc, hfont_new);
}

inline void Chess::render()
{
    game::util::draw_blackground(&this->g_mdc, screen_width, screen_height, WHITE_BRUSH);

    this->render_chess_board();
    this->render_chess_pieces();
    this->render_empty_chess();
    this->render_text();
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

inline void Chess::render_chess_board()
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
    Pos current_pos { chess_piece_width_idx, chess_piece_height_idx };
    bool pos_in_board = this->pos_in_board(current_pos);
    if (pos_in_board) {
		// If the click position is in the chess board, process the click
		if (this->game_model_idx == Chess::PAIR_MODEL) {
			this->game_model_pair(width_pos, height_pos);
		} else if (this->game_model_idx == Chess::AI_MODEL) {
			this->game_model_ai(width_pos, height_pos);
		} else if (this->game_model_idx == Chess::NET_MODEL) {
			this->game_model_web(width_pos, height_pos);
		}
    }
    this->check_game_data();
}

void Chess::game_model_pair(int width_pos, int height_pos)
{
    int offset_x_from_chess = width_pos - (screen_width / 2 - board_bmp_width / 2);
    int offset_y_from_chess = height_pos - (screen_height / 2 - board_bmp_height / 2);
    int chess_piece_height_idx = offset_y_from_chess / chess_piece_height;
    int chess_piece_width_idx = offset_x_from_chess / chess_piece_width;
    Pos current_pos { chess_piece_width_idx, chess_piece_height_idx };
    bool pos_in_board = this->pos_in_board(current_pos);
    if (pos_in_board) {
        ChessPieceType chess_piece_type = this->chess_board[chess_piece_height_idx][chess_piece_width_idx];
        bool has_selected_chess_piece = this->selected_pos != NONE_POS;
        if (has_selected_chess_piece == false) {
            this->selected_pos = current_pos;
            this->chess_target_pos_s = std::move(this->chess_piece_pos(current_pos));
        } else {
            bool in_target_pos = find(this->chess_target_pos_s.begin(), this->chess_target_pos_s.end(), current_pos) != this->chess_target_pos_s.end();
            if (in_target_pos) {
                ChessPieceType pre_chess_piece_type = this->chess_board[this->selected_pos.h_pos][this->selected_pos.w_pos];
                this->chess_board[current_pos.h_pos][current_pos.w_pos] = pre_chess_piece_type;
                this->chess_board[this->selected_pos.h_pos][this->selected_pos.w_pos] = ChessPieceType::empty;
                this->next_turn();
            }
            this->chess_target_pos_s.clear();
            this->selected_pos = NONE_POS;
        }
    }
}

void Chess::game_model_ai(int width_pos, int height_pos)
{
    int offset_x_from_chess = width_pos - (screen_width / 2 - board_bmp_width / 2);
    int offset_y_from_chess = height_pos - (screen_height / 2 - board_bmp_height / 2);
    int chess_piece_height_idx = offset_y_from_chess / chess_piece_height;
    int chess_piece_width_idx = offset_x_from_chess / chess_piece_width;
    Pos current_pos { chess_piece_width_idx, chess_piece_height_idx };
    bool pos_in_board = this->pos_in_board(current_pos);
    if (pos_in_board) {
        ChessPieceType chess_piece_type = this->chess_board[chess_piece_height_idx][chess_piece_width_idx];
        bool has_selected_chess_piece = this->selected_pos != NONE_POS;
        if (has_selected_chess_piece == false) {
            this->selected_pos = current_pos;
            if (this->is_red_turn()) {
                this->chess_target_pos_s = std::move(this->red_player_pos(chess_piece_width_idx, chess_piece_height_idx, chess_piece_type));
            }
        } else {
            bool in_target_pos = find(this->chess_target_pos_s.begin(), this->chess_target_pos_s.end(), current_pos) != this->chess_target_pos_s.end();
            if (in_target_pos) {
                ChessPieceType pre_chess_piece_type = this->chess_board[this->selected_pos.h_pos][this->selected_pos.w_pos];
                this->chess_board[current_pos.h_pos][current_pos.w_pos] = pre_chess_piece_type;
                this->chess_board[this->selected_pos.h_pos][this->selected_pos.w_pos] = ChessPieceType::empty;
                this->next_turn();
            }
            this->chess_target_pos_s.clear();
            this->selected_pos = NONE_POS;
        }
    }
}

void Chess::game_model_web(int width_pos, int height_pos)
{
    int offset_x_from_chess = width_pos - (screen_width / 2 - board_bmp_width / 2);
    int offset_y_from_chess = height_pos - (screen_height / 2 - board_bmp_height / 2);
    int chess_piece_height_idx = offset_y_from_chess / chess_piece_height;
    int chess_piece_width_idx = offset_x_from_chess / chess_piece_width;
    Pos current_pos { chess_piece_width_idx, chess_piece_height_idx };
    bool pos_in_board = this->pos_in_board(current_pos);
    if (pos_in_board) {
        ChessPieceType chess_piece_type = this->chess_board[chess_piece_height_idx][chess_piece_width_idx];
        bool has_selected_chess_piece = this->selected_pos != NONE_POS;
        WebOpponent* opponent = reinterpret_cast<WebOpponent*>(this->opponent.get());
        bool is_red = this->is_red_turn();
        bool is_my_turn = (is_red && !opponent->is_black_player()) || (!is_red && opponent->is_black_player());
        if (is_my_turn) {
            if (has_selected_chess_piece == false) {
                this->selected_pos = current_pos;
                if (red_turn) {
                    this->chess_target_pos_s = std::move(this->red_player_pos(chess_piece_width_idx, chess_piece_height_idx, chess_piece_type));
                } else {
                    this->chess_target_pos_s = std::move(this->black_player_pos(chess_piece_width_idx, chess_piece_height_idx, chess_piece_type));
                }
            } else {
                bool in_target_pos = find(this->chess_target_pos_s.begin(), this->chess_target_pos_s.end(), current_pos) != this->chess_target_pos_s.end();
                if (in_target_pos) {
                    struct Pos& source_pos = this->selected_pos;
                    struct Pos& target_pos = current_pos;
                    opponent->self_move(source_pos, target_pos);
                    ChessPieceType pre_chess_piece_type = this->chess_board[this->selected_pos.h_pos][this->selected_pos.w_pos];
                    this->chess_board[current_pos.h_pos][current_pos.w_pos] = pre_chess_piece_type;
                    this->chess_board[this->selected_pos.h_pos][this->selected_pos.w_pos] = ChessPieceType::empty;
                    this->next_turn();
                }
                this->chess_target_pos_s.clear();
                this->selected_pos = NONE_POS;
            }
        }
    }
}

void Chess::move_chess_piece(const Pos& source_pos, const Pos& target_pos)
{
    ChessPieceType source_chess_piece_type = this->get_piece_type(source_pos);
    std::vector<Pos> pos_s = std::move(this->chess_piece_pos(source_pos));
    bool in_pos_s_flag = std::find(pos_s.begin(), pos_s.end(), target_pos) != pos_s.end();
    if (in_pos_s_flag) {
        ChessPieceType pre_chess_piece_type = this->chess_board[source_pos.h_pos][source_pos.w_pos];
        this->chess_board[target_pos.h_pos][target_pos.w_pos] = pre_chess_piece_type;
        this->chess_board[source_pos.h_pos][source_pos.w_pos] = ChessPieceType::empty;
    }
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
    for (int idx = 0; idx < offsets_len; ++idx) {
        PosOffset& offset = offsets[idx];
        Pos source_pos { width_idx, height_idx };
        while (true) {
            source_pos += offset;
            bool pos_in_board_flag = Chess::pos_in_board(source_pos);
            if (pos_in_board_flag == false) {
                break;
            }

            ChessPieceType type = this->chess_board[source_pos.h_pos][source_pos.w_pos];
            if (type == ChessPieceType::empty) {
                ret.push_back(source_pos);
            } else {
                while (true) {
                    source_pos += offset;
                    bool pos_in_board_flag = Chess::pos_in_board(source_pos);
                    if (pos_in_board_flag == false) {
                        break;
                    }
                    ChessPieceType type = this->chess_board[source_pos.h_pos][source_pos.w_pos];

                    if (this->is_empty(type) == false) {
                        bool is_enemy_flag = this->is_enemy(type, cannon_type);
                        if (is_enemy(type, cannon_type)) {
                            ret.push_back(source_pos);
                        }
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
    for (int idx = 0; idx < offsets_len; ++idx) {
        PosOffset& offset = offsets[idx];
        Pos source_pos { width_idx, height_idx };
        while (true) {
            source_pos += offset;
            bool pos_in_board_flag = this->pos_in_board(source_pos);
            if (pos_in_board_flag == false) {
                break;
            }

            ChessPieceType type = this->chess_board[source_pos.h_pos][source_pos.w_pos];
            if (type == ChessPieceType::empty) {
                ret.push_back(source_pos);
            } else {
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
        { 1, 2 },
        { 1, -2 },
        { -1, 2 },
        { -1, -2 }
    };
    Pos source_pos(width_idx, height_idx);
    int offsets_len = array_length(offsets);
    for (int idx = 0; idx < offsets_len; ++idx) {
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
        { 2, 2 },
        { 2, -2 },
        { -2, 2 },
        { -2, -2 }
    };
    int offsets_len = array_length(offsets);
    Pos source_pos(width_idx, height_idx);
    std::vector<Pos> ret;
    for (int idx = 0; idx < offsets_len; idx++) {
        PosOffset& offset = offsets[idx];
        Pos target_pos = source_pos + offset;
        if (this->pos_in_board(target_pos)) {
            ChessPieceType chess_piece_type = this->get_piece_type(target_pos);
            if (this->is_enemy(elephant_type, chess_piece_type) || this->is_empty(chess_piece_type)) {
                if (is_red(elephant_type) && this->in_red_area(target_pos)) {
                    ret.push_back(target_pos);
                } else if (is_black(elephant_type) && this->in_black_area(target_pos)) {
                    ret.push_back(target_pos);
                }
            }
        }
    }
    return ret;
}

std::vector<Pos> Chess::guard_move(int width_idx, int height_idx, ChessPieceType guard_type)
{
    std::vector<Pos> ret;
    PosOffset offsets[] = {
        { 1, 1 },
        { 1, -1 },
        { -1, 1 },
        { -1, -1 }
    };
    Pos source_pos = Pos(width_idx, height_idx);
    int offset_len = array_length(offsets);
    for (int idx = 0; idx < offset_len; ++idx) {
        const PosOffset& pos_offset = offsets[idx];
        Pos target_pos = source_pos + pos_offset;
        if (this->pos_in_board(target_pos)) {
            ChessPieceType target_piece_type = this->get_piece_type(target_pos);
            if (this->is_empty(target_piece_type) || this->is_enemy(target_piece_type, guard_type)) {
                if (this->is_red(guard_type) && this->in_red_king_area(target_pos)) {
                    ret.push_back(target_pos);
                } else if (this->is_black(guard_type) && this->in_black_king_area(target_pos)) {
                    ret.push_back(target_pos);
                }
            }
        }
    }
    return ret;
}

std::vector<Pos> Chess::king_move(int width_idx, int height_idx, ChessPieceType king_type)
{
    std::vector<Pos> ret;
    PosOffset offsets[] = {
        { 1, 0 },
        { -1, 0 },
        { 0, 1 },
        { 0, -1 }
    };
    int length = array_length(offsets);
    Pos source_pos(width_idx, height_idx);
    for (int idx = 0; idx < length; ++idx) {
        const PosOffset& pos_offset = offsets[idx];
        Pos target_pos = source_pos + pos_offset;
        if (this->pos_in_board(target_pos)) {
            ChessPieceType target_piece_type = this->get_piece_type(target_pos);
            if (this->is_empty(target_piece_type) || this->is_enemy(target_piece_type, king_type)) {
                if (this->is_red(king_type) && this->in_red_king_area(target_pos)) {
                    ret.push_back(target_pos);
                } else if (this->is_black(king_type) && this->in_black_king_area(target_pos)) {
                    ret.push_back(target_pos);
                }
            }
        }
    }
    return ret;
}

std::vector<Pos> Chess::soldier_move(int width_idx, int height_idx, ChessPieceType soldier_type)
{
    std::vector<Pos> pos_s;
    Pos source_pos(width_idx, height_idx);
    if (is_black(soldier_type)) {

        if (this->in_black_area(source_pos)) {
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
    } else if (is_red(soldier_type)) {

        if (in_red_area(source_pos)) {
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
    }
    return pos_s;
}

std::vector<Pos> Chess::black_player_pos(int chess_piece_width_idx, int chess_piece_height_idx, ChessPieceType chess_piece_type)
{
    std::vector<Pos> pos_s;
    if (chess_piece_type == ChessPieceType::black_soldier) {
        pos_s = std::move(this->soldier_move(chess_piece_width_idx, chess_piece_height_idx, ChessPieceType::black_soldier));
    } else if (chess_piece_type == ChessPieceType::black_cannon) {
        pos_s = std::move(this->cannon_move(chess_piece_width_idx, chess_piece_height_idx, ChessPieceType::black_cannon));
    } else if (chess_piece_type == ChessPieceType::black_rook) {
        pos_s = std::move(this->rook_move(chess_piece_width_idx, chess_piece_height_idx, ChessPieceType::black_rook));
    } else if (chess_piece_type == ChessPieceType::black_knight) {
        pos_s = std::move(this->knight_move(chess_piece_width_idx, chess_piece_height_idx, ChessPieceType::black_knight));
    } else if (chess_piece_type == ChessPieceType::black_elephant) {
        pos_s = std::move(this->elephant_move(chess_piece_width_idx, chess_piece_height_idx, ChessPieceType::black_elephant));
    } else if (chess_piece_type == ChessPieceType::black_guard) {
        pos_s = std::move(this->guard_move(chess_piece_width_idx, chess_piece_height_idx, ChessPieceType::black_elephant));
    } else if (chess_piece_type == ChessPieceType::black_king) {
        pos_s = std::move(this->king_move(chess_piece_width_idx, chess_piece_height_idx, ChessPieceType::black_king));
    }
    return pos_s;
}

std::vector<Pos> Chess::red_player_pos(int chess_piece_width_idx, int chess_piece_height_idx, ChessPieceType chess_piece_type)
{
    std::vector<Pos> pos_s;
    if (chess_piece_type == ChessPieceType::red_soldier) {
        pos_s = std::move(this->soldier_move(chess_piece_width_idx, chess_piece_height_idx, ChessPieceType::red_soldier));
    } else if (chess_piece_type == ChessPieceType::red_cannon) {
        pos_s = std::move(this->cannon_move(chess_piece_width_idx, chess_piece_height_idx, ChessPieceType::red_cannon));
    } else if (chess_piece_type == ChessPieceType::red_rook) {
        pos_s = std::move(this->rook_move(chess_piece_width_idx, chess_piece_height_idx, ChessPieceType::red_rook));
    } else if (chess_piece_type == ChessPieceType::red_knight) {
        pos_s = std::move(this->knight_move(chess_piece_width_idx, chess_piece_height_idx, ChessPieceType::red_knight));
    } else if (chess_piece_type == ChessPieceType::red_elephant) {
        pos_s = std::move(this->elephant_move(chess_piece_width_idx, chess_piece_height_idx, ChessPieceType::red_elephant));
    } else if (chess_piece_type == ChessPieceType::red_guard) {
        pos_s = std::move(this->guard_move(chess_piece_width_idx, chess_piece_height_idx, ChessPieceType::red_guard));
    } else if (chess_piece_type == ChessPieceType::red_king) {
        pos_s = std::move(this->king_move(chess_piece_width_idx, chess_piece_height_idx, ChessPieceType::red_king));
    }
    return pos_s;
}

std::vector<Pos> Chess::chess_piece_pos(const Pos& pos)
{
    std::vector<Pos> pos_s;
    ChessPieceType chess_piece_type = this->get_piece_type(pos);
    if (this->is_red_turn() == false) {
        pos_s = std::move(this->chess_target_pos_s = std::move(this->black_player_pos(pos.w_pos, pos.h_pos, chess_piece_type)));
    } else if (this->is_red_turn()) {
        pos_s = std::move(this->chess_target_pos_s = std::move(this->red_player_pos(pos.w_pos, pos.h_pos, chess_piece_type)));
    }
    return pos_s;
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
    return Chess::pos_in_board(pos.w_pos, pos.h_pos);
}

inline bool Chess::pos_in_board(int width_idx, int height_idx)
{
    bool ret = (width_idx >= 0 && width_idx < Chess::board_width && height_idx >= 0 && height_idx < Chess::board_height);
    return ret;
}

PosOffset Chess::get_knight_foot(PosOffset pos)
{
    int width_offset = pos.width_offset, height_offset = pos.height_offset;
    PosOffset pos_offset { -1, -1 };
    if (abs(width_offset) == 2) {
        pos_offset = PosOffset(width_offset / 2, 0);
    } else if (abs(height_offset) == 2) {
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
}

void Chess::render_text()
{
    int text_width = board_bmp_width + 50;
    int test_height = board_bmp_height / 2;
    SetTextColor(g_mdc, RGB(255, 0, 0));
    SetBkMode(g_mdc, TRANSPARENT);
    const TCHAR* text = this->is_red_turn() ? TEXT("红方回合") : TEXT("黑方回合");
    TextOut(g_mdc, text_width, test_height, text, lstrlen(text));
}
Opponent::Opponent(Chess* chess_ptr)
{
    this->chess_ptr = chess_ptr;
}
void AIOpponent::move()
{
    if (this->chess_ptr->is_red_turn() == false) {

		int best_score = INT_MIN;
		Pos best_from, best_to;

		auto all_moves = this->get_all_moves(true); // 黑方 AI
		for (const auto& move : all_moves) {
			Chess::ChessPieceType captured_piece;
			make_move(move.first, move.second, captured_piece);
			int score = minimax(3, false); // 深度为 3
			undo_move(move.first, move.second, captured_piece);

			if (score > best_score) {
				best_score = score;
				best_from = move.first;
				best_to = move.second;
			}
		}

		// 执行最佳移动
		chess_ptr->move_chess_piece(best_from, best_to);
        this->chess_ptr->next_turn();
    }
}

int AIOpponent::evaluate_board()
{
    int ret { 0 };
    bool black_king_flag = false, red_king_flag = false;
    for (int h = 0; h < Chess::board_height; ++h) {
        for (int w = 0; w < Chess::board_width; ++w) {
            Pos pos { w, h };
            Chess::ChessPieceType chess_piece = this->chess_ptr->get_piece_type(pos);
            switch (chess_piece) {
            case Chess::ChessPieceType::black_king: {
                black_king_flag = true;
                break;
            }
            case Chess::ChessPieceType::black_cannon: {
                ret += 7;
                if (this->chess_ptr->in_red_area(pos)) {
                    ret += 3;
                }
                break;
            }
            case Chess::ChessPieceType::black_elephant:
                ret += 3;
                break;
            case Chess::ChessPieceType::black_guard:
                ret += 2;
                break;
            case Chess::ChessPieceType::black_knight: {
                ret += 5;
                if (this->chess_ptr->in_red_area(pos)) {
                    ret += 3;
                }
                break;
            }
            case Chess::ChessPieceType::black_rook: {
                ret += 10;
                if (this->chess_ptr->in_red_area(pos)) {
                    ret += 3;
                }
                break;
            }
            case Chess::ChessPieceType::black_soldier: {
                ret += 3;
                if (this->chess_ptr->in_black_area(pos)) {
                    ret += 5;
                }
                break;
            }
            case Chess::ChessPieceType::red_king: {
                red_king_flag = true;
                break;
            }
            case Chess::ChessPieceType::red_cannon: {
                ret -= 7;
                if (this->chess_ptr->in_black_area(pos)) {
                    ret -= 3;
                }
                break;
            }
            case Chess::ChessPieceType::red_elephant:
                ret -= 3;
                break;
            case Chess::ChessPieceType::red_guard:
                ret -= 2;
                break;
            case Chess::ChessPieceType::red_knight: {
                ret -= 5;
                if (this->chess_ptr->in_black_area(pos)) {
                    ret -= 3;
                }
                break;
            }
            case Chess::ChessPieceType::red_rook: {
                ret -= 10;
                if (this->chess_ptr->in_black_area(pos)) {
                    ret -= 3;
                }
                break;
            }
            case Chess::ChessPieceType::red_soldier:
                ret -= 3;
                if (this->chess_ptr->in_black_area(pos)) {
                    ret -= 5;
                }
                break;
            }
        }
    }
    if (black_king_flag == false) {
        ret = INT_MIN;
    } else if (red_king_flag == false) {
        ret = INT_MAX;
    }
    return ret;
}

inline void AIOpponent::make_move(const Pos& from, const Pos& to, Chess::ChessPieceType& captured_piece)
{
    captured_piece = this->chess_ptr->get_piece_type(to);
    this->chess_ptr->set_piece_type(to, this->chess_ptr->get_piece_type(from));
    this->chess_ptr->set_piece_type(from, Chess::ChessPieceType::empty);
}

inline void AIOpponent::undo_move(const Pos& from, const Pos& to, Chess::ChessPieceType captured_piece)
{
    this->chess_ptr->set_piece_type(from, this->chess_ptr->get_piece_type(to));
    this->chess_ptr->set_piece_type(to, captured_piece);
}

std::vector<std::pair<Pos, Pos>> AIOpponent::get_all_moves(bool is_black)
{
    std::vector<std::pair<Pos, Pos>> ret;
    for (int h = 0; h < Chess::board_height; ++h) {
        for (int w = 0; w < Chess::board_width; ++w) {
            Chess::ChessPieceType piece_type = this->chess_ptr->get_piece_type({ w, h });
            std::vector<Pos> pos_s;
            if ((is_black && this->chess_ptr->is_black(piece_type))) {
                pos_s = std::move(this->chess_ptr->black_player_pos(w, h, piece_type));
            } else if ((is_black == false && this->chess_ptr->is_red(piece_type))) {
                pos_s = std::move(this->chess_ptr->red_player_pos(w, h, piece_type));
            }
            for (const auto& pos : pos_s) {
                ret.emplace_back(Pos { w, h }, pos);
            }
        }
    }
    return ret;
}

int AIOpponent::minimax(int depth, bool is_maximizing)
{
    if (depth == 0) {
        return evaluate_board();
    }

    int best_score = is_maximizing ? INT_MIN : INT_MAX;
    auto all_moves = get_all_moves(is_maximizing);

    for (const auto& move : all_moves) {
        Chess::ChessPieceType captured_piece;
        const Pos &source = move.first, &target = move.second;
        make_move(source, target, captured_piece);
        int score = minimax(depth - 1, !is_maximizing);
        undo_move(source, target, captured_piece);

        if (is_maximizing) {
            best_score = max(best_score, score);
        } else {
            best_score = min(best_score, score);
        }
    }

    return best_score;
}

WebOpponent::WebOpponent(Chess* ptr, int room_id, std::shared_ptr<Server> server)
    : Opponent(ptr)
{
    this->chess_ptr = ptr;
    this->room_id = room_id;
    this->server = server;
    this->black_player = true;
}

WebOpponent::EnterRoomRsp WebOpponent::enter_room()
{
    using json = nlohmann::json;

    json j_root;
    j_root["function"] = "enter_room";
    json param;
    param["room_id"] = room_id;
    param["black_player"] = black_player;
    j_root["param"] = param;
    std::string json_str = j_root.dump(-1);
    std::string msg = this->server->call(json_str);

    json j = json::parse(msg);

    EnterRoomRsp rsp;
    rsp.code = j["code"].get<int>();
    rsp.message = j["message"].get<std::string>();
    rsp.red_player = j["red_player"].get<bool>();

    this->black_player = !rsp.red_player;
    this->has_move = this->black_player;
    return rsp;
}

WebOpponent::LeaveRoomRsp WebOpponent::leave_room()
{
    using json = nlohmann::json;
    json j_room;
    j_room["function"] = "leave_room";
    json param;
    param["room_id"] = this->room_id;
    param["red_player"] = !this->black_player;
    j_room["param"] = param;

    std::string json_str = j_room.dump(-1);
    std::string rsp = this->server->call(json_str);

    json j = json::parse(rsp);

    LeaveRoomRsp leave_rsp;
    leave_rsp.code = j["code"].get<int>();
    leave_rsp.message = j["message"].get<std::string>();
    return leave_rsp;
}

void WebOpponent::move()
{
    if (has_move == true) {

        int code = 1;
        do {

            using json = nlohmann::json;
            json j_room;
            j_room["function"] = "move";
            json param;
            param["room_id"] = this->room_id;
            param["red_player"] = !this->black_player;
            j_room["param"] = param;

            std::string json_str = j_room.dump(-1);
            std::string rsp = this->server->call(json_str);

            json j = json::parse(rsp);

            code = j["code"].get<int>();
            if (code == 1) {
                struct Pos source_pos = j["source_pos"].get<struct Pos>();
                struct Pos target_pos = j["target_pos"].get<struct Pos>();

                Chess::ChessPieceType pre_chess_piece_type = chess_ptr->get_piece_type(source_pos);
                this->chess_ptr->set_piece_type(target_pos, pre_chess_piece_type);
                this->chess_ptr->set_piece_type(source_pos, Chess::ChessPieceType::empty);
                has_move = false;
                this->chess_ptr->next_turn();
            }
            Sleep(1000);
        } while (code != 1);
    }
}

void WebOpponent::set_black_player(bool black_player)
{
    this->black_player = black_player;
}

bool WebOpponent::is_black_player()
{
    return this->black_player;
}

void WebOpponent::self_move(Pos& source_pos, Pos& target_pos)
{
    using json = nlohmann::json;
    json j_room;
    j_room["function"] = "self_move";
    json param;
    param["room_id"] = this->room_id;
    param["red_player"] = !this->black_player;
    param["source_pos"] = source_pos;
    param["target_pos"] = target_pos;
    j_room["param"] = param;

    std::string json_str = j_room.dump(-1);
    std::string rsp = this->server->call(json_str);

    json j = json::parse(rsp);

    LeaveRoomRsp leave_rsp;
    leave_rsp.code = j["code"].get<int>();
    leave_rsp.message = j["message"].get<std::string>();
    has_move = true;
}

void PairOpponent::move()
{
}
