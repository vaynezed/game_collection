#include <algorithm>
#include <windowsx.h>
#include <windows.h>
#include <commctrl.h>



#include "games/pool.hpp"
#include "base/log.hpp"
#include "games/utils.hpp"

void Pool::game_init(HWND& hwnd)
{
	g_hdc = GetDC(hwnd);
	this->g_mdc = CreateCompatibleDC(g_hdc);
	this->g_bufdc = CreateCompatibleDC(g_hdc);
	HBITMAP bmp = CreateCompatibleBitmap(g_hdc, screen_width, screen_height);
	main_hwnd_ptr = &hwnd;
	SelectObject(g_mdc, bmp);
	DeleteObject(bmp);
	this->g_pen = CreatePen(PS_SOLID, 10, RGB(255, 0, 0)); // 红色实线，宽度为2

	this->init_balls();
	this->init_holes();
	this->init_walls();
	this->game_init_flag = true;
	this->log_ptr = Log::get_logger("main", "main.log");
}

// 计算两点之间的距离
inline double distance(const Point& p1, const Point& p2) {
	return sqrt((p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y));
}

// 计算点在直线上的投影
Point projectPoint(const Line& ls, const Point& p) {
	int dx = ls.end.x - ls.start.x;
	int dy = ls.end.y - ls.start.y;
	int lenSq = dx * dx + dy * dy;
	if (lenSq == 0) return ls.start; // 线段长度为0

	double t = static_cast<double>((p.x - ls.start.x) * dx + (p.y - ls.start.y) * dy) / lenSq;
	t = max(0.0, min(1.0, t));

	return { ls.start.x + t * dx, ls.start.y + t * dy};
}

// 判断线段和圆是否相交
bool Ball::isIntersect(const Line& ls) {
	Point pos{ this->pos.x, this->pos.y};
	Point proj = projectPoint(ls, pos);
	double dist = distance(proj, pos);
	bool ret = dist <= this->r;
	return ret;
}

bool Ball::isIntersect(const Ball& other)
{
	double dx = this->pos.x - other.pos.x;
	double dy = this->pos.y - other.pos.y;
	double dz = sqrt(dx * dx + dy * dy);
	bool ret = dz < (this->r + other.r);
	return ret;
}

void Ball::update(ULONGLONG time, double f)
{
	double dt = static_cast<double>(time) / 1000;
	double dx = this->v.x * dt;
	double dy = this->v.y * dt;
	this->pos.y += dy;
	this->pos.x += dx;
	double dv = f * dt;
	if (abs(this->v.x) > dv) {
		if (this->v.x > 0) {
			this->v.x -= dv;
		}
		else {
			this->v.x += dv;
		}
	}
	else {
		this->v.x = 0;
	}

	if (abs(this->v.y) > dv) {
		if (this->v.y > 0) {
			this->v.y -= dv;
		}
		else {
			this->v.y += dv;
		}
	}
	else {
		this->v.y = 0;
	}
}

void Ball::hit_ball(Ball& other_ball)
{
	if (!(other_ball.visible && other_ball.enable && this->isIntersect(other_ball)))
		return;

	// 计算两球中心距离
	double dx = this->pos.x - other_ball.pos.x;
	double dy = this->pos.y - other_ball.pos.y;
	double dist = sqrt(dx * dx + dy * dy);
	if (dist == 0) return; // 防止除零

	// 归一化碰撞法线
	double nx = dx / dist;
	double ny = dy / dist;

	// 相对速度
	double dvx = this->v.x - other_ball.v.x;
	double dvy = this->v.y - other_ball.v.y;

	// 速度在碰撞法线上的分量
	double relVel = dvx * nx + dvy * ny;
	if (relVel > 0) return; // 已经分开，不再碰撞

	// 假设质量相等，弹性碰撞
	double impulse = -2 * relVel / 2;

	this->v.x += impulse * nx;
	this->v.y += impulse * ny;
	other_ball.v.x -= impulse * nx;
	other_ball.v.y -= impulse * ny;

	// 防止球重叠，简单分开
	double overlap = (this->r + other_ball.r) - dist;
	if (overlap > 0) {
		this->pos.x += nx * (overlap / 2);
		this->pos.y += ny * (overlap / 2);
		other_ball.pos.x -= nx * (overlap / 2);
		other_ball.pos.y -= ny * (overlap / 2);
	}
}

void Pool::game_loop()
{
	g_tnow = GetTickCount64();
	ULONGLONG dt = g_tnow - g_tpre;
	bool need_re_rend{ dt >= 10};
	if (need_re_rend) {
		update(dt);
		this->render();
		g_tpre = GetTickCount64();
	}

}

void Pool::game_cleanup()
{
	DeleteObject(this->g_pen);
	DeleteObject(this->cue_bmp);
	DeleteObject(this->triangle_bmp);
	DeleteObject(this->table_bmp);
	size_t len = array_length(balls);
	for (int idx = 0;idx < len;++idx) {
		DeleteObject(balls[idx].hbmp);
	}
	DeleteDC(this->g_bufdc);
	DeleteDC(this->g_mdc);
	ReleaseDC(*this->main_hwnd_ptr, g_hdc);

	this->game_init_flag = false;
	this->main_hwnd_ptr = nullptr;
}

std::wstring Pool::to_string()
{
	return std::wstring(TEXT("桌球"));
}

std::vector<std::wstring> Pool::game_models()
{
	return std::vector<std::wstring>();
}

void Pool::set_game_model(int idx)
{
}

void Pool::game_process_key_down(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(*this->main_hwnd_ptr, GWLP_HINSTANCE);
	switch (message) {
	case WM_KEYDOWN:
		switch (wParam) {
		case VK_ESCAPE: {
			INT_PTR ret = DialogBox(hInstance, MAKEINTRESOURCE(ID_RESTART_DLG), *main_hwnd_ptr, default_dlg_wnd_proc);
			if (ret == ID_GOBACK) {
				this->goback_main_menu();
			}
			else if (ret == ID_RESTART_BTN) {
			}
			break;
		}
		}
		break;
	case WM_LBUTTONDOWN: {
		int width_pos = GET_X_LPARAM(lParam);
		int height_pos = GET_Y_LPARAM(lParam);
		this->left_click_down(width_pos, height_pos);
		break;
	}
	case WM_LBUTTONUP: {
		int width_pos = GET_X_LPARAM(lParam);
		int height_pos = GET_Y_LPARAM(lParam);
		this->left_click_up(width_pos, height_pos);
		break;
	}
	case WM_MOUSEMOVE: {
		int width_pos = GET_X_LPARAM(lParam);
		int height_pos = GET_Y_LPARAM(lParam);
		this->mouse_move(width_pos, height_pos);
		break;
	}
	case WM_RBUTTONDOWN: {
		int width_pos = GET_X_LPARAM(lParam);
		int height_pos = GET_Y_LPARAM(lParam);
		this->right_click_down(width_pos, height_pos);
		break;
	}
	case WM_RBUTTONUP: {
		int width_pos = GET_X_LPARAM(lParam);
		int height_pos = GET_Y_LPARAM(lParam);
		this->right_click_up(width_pos, height_pos);
		break;
	}
	}

}

void Pool::render()
{
	game::util::draw_blackground(&this->g_mdc, screen_width, screen_height, WHITE_BRUSH);
	SelectObject(this->g_bufdc, this->table_bmp);
	BitBlt(this->g_mdc, 0 ,0 , screen_width, screen_height, this->g_bufdc, 0 ,0 ,SRCCOPY);

	this->render_balls();
	if (left_mouse_flag == true) {
		SelectObject(this->g_mdc, this->g_pen);
		MoveToEx(this->g_mdc, this->start_pos.w_pos, this->start_pos.h_pos, NULL);
		LineTo(this->g_mdc, this->end_pos.w_pos, this->end_pos.h_pos);
	}
	if (this->left_mouse_flag) {
		this->render_progress_bar();
	}
	BitBlt(this->g_hdc, 0, 0, screen_width, screen_height, this->g_mdc, 0, 0, SRCCOPY);
}

void Pool::render_balls()
{
	size_t len = array_length(balls);
	for (int idx = 0;idx < len;++idx) {
		Ball& ball = balls[idx];
		ball.render(&this->g_bufdc, &this->g_mdc);
	}
}

void Pool::render_progress_bar()
{
	RECT rect{ end_pos.w_pos, end_pos.h_pos, end_pos.w_pos + 100, end_pos.h_pos + 20};
	FillRect(this->g_mdc, &rect, (HBRUSH)GetStockObject(LTGRAY_BRUSH));
	ULONGLONG now = GetTickCount64();
	int step = (now - this->r_tpre) / 10;
	if (this->right_mouse_flag) {
		 if (step > 0) {
			 this->progress = min(this->progress + step, 100);
			 this->r_tpre = GetTickCount64();
		 }
	}
	else {
		 if (step > 0) {
			 this->progress = max(this->progress - step, 0);
			 this->r_tpre = GetTickCount64();
		 }

	}
	RECT fillRect = rect;
	fillRect.right = rect.left + MulDiv(progress, rect.right - rect.left, 100);
	FillRect(this->g_mdc, &fillRect, CreateSolidBrush(RGB(255, 0, 0)));

}

void Pool::update(ULONGLONG dt)
{
	const size_t len = array_length(balls);

	// 1. 处理球与球之间的碰撞
	for (size_t i = 0; i < len; ++i) {
		Ball& ball1 = balls[i];
		if (ball1.visible && ball1.enable) {
			for (size_t j = i + 1; j < len; ++j) {
				Ball& ball2 = balls[j];
				if (ball2.visible && ball2.enable) {
					ball1.hit_ball(ball2);
				}
			}
		}
	}

	// 2. 处理每个球的进洞、碰墙和移动
	for (size_t i = 0; i < len; ++i) {
		Ball& ball = balls[i];
		if (ball.visible && ball.enable) {
			if (abs(ball.v.x) > 1e-5 || abs(ball.v.y) > 1e-5) {
				if (this->hit_hole(ball)) {
					ball.visible = false;
					ball.enable = false;
					continue;
				}
				this->hit_wall(ball);
				ball.update(dt, 50.0);
			}
		}
	}
}


void Pool::init_balls()
{
	std::string folder_name("./resource/pool/");
	for (int idx = 1;idx <= 16;idx++) {
		std::string filename = std::string("ball_") + std::to_string(idx) + ".bmp";
		std::string path = folder_name + filename;
		Ball& ball = balls[idx - 1];
		ball.hbmp = (HBITMAP)LoadImageA(nullptr, path.c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		ball.num = idx;
		ball.v = Vec(0, 0);
		ball.visible = true;
		ball.enable = true;

		if (!ball.hbmp) {
			throw std::runtime_error("Failed to load bitmap: " + path);
		}
	}

	size_t len = array_length(balls);
	int center_screen_width_pos = screen_width / 2;
	int center_screen_height_pos = screen_height / 2;
	int first_ball_width_pos = center_screen_width_pos - 2.5 * BALL_BMP_W;
	int first_ball_height_pos = center_screen_height_pos - 2.5 * BALL_BMP_H;

	int idx = 0;
	for (int width_idx = 0; width_idx < 5;++width_idx) {
		int top = first_ball_height_pos + BALL_BMP_H * (width_idx * 0.5);
		int left = first_ball_width_pos + width_idx * BALL_BMP_W;
		int height_len = 5 - width_idx;
		for (int height_idx = 0;height_idx < height_len;++height_idx) {
			int width_pos = left;
			int height_pos = top + height_idx * BALL_BMP_H;
			Ball& ball = balls[idx];

			ball.pos = Point(width_pos +BALL_BMP_W /2, height_pos+ BALL_BMP_H / 2);
			ball.r =  BALL_BMP_W/ 2;
			++idx;
		}
	}

	Ball& white_ball = balls[len - 1];
	white_ball.pos = Point(center_screen_width_pos * 1.5, center_screen_height_pos);
	white_ball.r = BALL_BMP_W / 2;


	std::string desktop_file_name{"table.bmp" };
	std::string desktop_path = folder_name + desktop_file_name;
	this->table_bmp = (HBITMAP)LoadImageA(nullptr, desktop_path.c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	if (!this->table_bmp) {
		throw std::runtime_error("Failed to load bitmap: " + desktop_path);
	}
	std::string triangle_bmp_file_name{"triangle.bmp" };
	std::string triangle_bmp_path = folder_name + triangle_bmp_file_name;
	this->triangle_bmp = (HBITMAP)LoadImageA(nullptr, triangle_bmp_path.c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	if (!this->table_bmp) {
		throw std::runtime_error("Failed to load bitmap: " + triangle_bmp_path);
	}

	std::string cue_bmp_file_name{"cue.bmp"};
	std::string cue_bmp_path = folder_name + cue_bmp_file_name;
	this->cue_bmp = (HBITMAP)LoadImageA(nullptr,cue_bmp_path.c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	if (!this->table_bmp) {
		throw std::runtime_error("Failed to load bitmap: " + cue_bmp_path);
	}
}

void Pool::left_click_down(int width_pos, int height_pos)
{
	if (left_mouse_flag == false) {
		left_mouse_flag = true;
		this->start_pos = Pos(width_pos, height_pos);
	}
}

void Pool::left_click_up(int width_pos, int height_pos)
{
	this->left_mouse_flag = false;
	this->cue_ball();
}

void Pool::right_click_up(int width_pos, int height_pos)
{
	this->right_mouse_flag = false;
}

void Pool::right_click_down(int width_pos, int height_pos)
{
	this->r_tpre = GetTickCount64();
	this->right_mouse_flag = true;
}

void Pool::mouse_move(int width_pos, int height_pos)
{
	this->end_pos = Pos(width_pos, height_pos);
}

void Pool::cue_ball()
{
	int h_offset = (end_pos.h_pos - start_pos.h_pos);
	int w_offset = (end_pos.w_pos - start_pos.w_pos);
	double  r =sqrt(h_offset * h_offset + w_offset * w_offset);
	double sin_v = static_cast<double>(h_offset) / r;
	double cos_v = static_cast<double>(w_offset) / r;
	int w_len = progress * cos_v;
	int h_len = progress * sin_v;
	int target_w = end_pos.w_pos + w_len;
	int target_h = end_pos.h_pos + h_len;
	Line line(Point(start_pos.w_pos, start_pos.h_pos), Point(target_w, target_h));
	size_t len = array_length(balls);
	Ball& ball = balls[len -1 ];
	char buf[1024];
	bool intersect = ball.isIntersect(line);
	if (intersect) {
		ball.v.x = cos_v * progress * 5;
		ball.v.y = sin_v * progress * 5;
	}
}

void Pool::hit_wall(Ball& ball)
{
	Point& pos = ball.pos;
	if ((point_in_rect(pos, this->top_wall[0]) || point_in_rect(pos, this->top_wall[1])) && ball.v.y < 0) {
		ball.v.y = -ball.v.y;
	}
	else if ((point_in_rect(pos, this->bottom_wall[0]) || point_in_rect(pos, this->bottom_wall[1])) && ball.v.y > 0) {
		ball.v.y = -ball.v.y;
	}

	if (point_in_rect(pos, this->left_wall) && ball.v.x < 0) {
		ball.v.x = -ball.v.x;
	}
	else if (point_in_rect(pos, this->right_wall) && ball.v.x > 0) {
		ball.v.x = -ball.v.x;
	}

}

bool Pool::hit_hole(Ball& ball)
{
	bool ret = false;
	Point& point = ball.pos;
	size_t len = array_length(holes);
	RECT ball_rect{ point.x - ball.r, point.y - ball.r, point.x + ball.r ,point.y + ball.r };
	for (int idx = 0;idx < len;++idx) {
		const RECT& hale_rect = this->holes[idx];
		if (rect_in_rect(ball_rect, hale_rect)) {
			this->log_ptr->info("hit hole");
			ret = true;
		}
	}
	return ret;
}

void Pool::init_holes()
{
	int half_table_width = (screen_width - 2 * WALL_WITDH - 100) / 2;
	this->holes[0].top = 50;
	this->holes[0].bottom = 150;
	this->holes[0].left = 40;
	this->holes[0].right = 140;

	this->holes[1].top = 25;
	this->holes[1].bottom = 125;
	this->holes[1].left = 900;
	this->holes[1].right = 1000;

	this->holes[2].top = 50;
	this->holes[2].bottom = 150;
	this->holes[2].left = 1765;
	this->holes[2].right = 1865;

	this->holes[3].top = 935;
	this->holes[3].bottom = 1035;
	this->holes[3].left = 40;
	this->holes[0].right = 140;

	this->holes[4].top = 960;
	this->holes[4].bottom =1060;
	this->holes[4].left = 900;
	this->holes[4].right = 1000;

	this->holes[5].top = 935;
	this->holes[5].bottom = 1035;
	this->holes[5].left = 1765;
	this->holes[5].right = 1865;

}

void Pool::init_walls()
{
	int half_table_width = (screen_width - 2 * WALL_WITDH - 100) / 2;
	this->top_wall[0].left = WALL_WITDH;
	this->top_wall[0].right = WALL_WITDH + half_table_width;
	this->top_wall[0].top = 0;
	this->top_wall[0].bottom = WALL_HEIGHT;

	this->top_wall[1].left = WALL_WITDH + half_table_width + 100;
	this->top_wall[1].right = this->top_wall[1].left + half_table_width;
	this->top_wall[1].top = 0;
	this->top_wall[1].bottom = WALL_HEIGHT;

	this->bottom_wall[0].left = WALL_WITDH;
	this->bottom_wall[0].right = WALL_WITDH + half_table_width;
	this->bottom_wall[0].top = screen_height - WALL_HEIGHT;
	this->bottom_wall[0].bottom = screen_height;

	this->bottom_wall[1].left = WALL_WITDH + half_table_width + 100;
	this->bottom_wall[1].right = this->bottom_wall[1].left + half_table_width;
	this->bottom_wall[1].top = screen_height - WALL_HEIGHT;
	this->bottom_wall[1].bottom = screen_height;

	this->left_wall.left = 0;
	this->left_wall.right = WALL_WITDH;
	this->left_wall.top = WALL_HEIGHT;
	this->left_wall.bottom = screen_height - WALL_HEIGHT;

	this->right_wall.left = screen_width - WALL_WITDH;
	this->right_wall.right = screen_width;
	this->right_wall.top = WALL_HEIGHT;
	this->right_wall.bottom = screen_height - WALL_HEIGHT;
}

void Ball::render(HDC* bufdc, HDC* target_dc)
{
	if (this->visible) {
		int left = this->pos.x- r;
		int top = this->pos.y- r;
		SelectObject(*bufdc, this->hbmp);
		int d = r * 2;
		TransparentBlt(*target_dc, left, top, d,d,
			*bufdc, 0, 0, d, d, RGB(0, 0, 0));
	}
}
bool rect_in_rect(const RECT& lhs, const RECT& rhs) {
	return lhs.left >= rhs.left && lhs.right <= rhs.right && lhs.top >= rhs.top && lhs.bottom <= rhs.bottom;
}

bool point_in_rect(const Point& point,const  RECT& rect)
{
	bool ret = point.x >= rect.left && point.x <= rect.right && point.y >= rect.top && point.y <= rect.bottom;
	return ret;
}
