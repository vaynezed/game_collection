#pragma once

#include "game.hpp"
#include "base/log.hpp"

struct Vec {
	double y; // Y-axis
	double x;  // X-axis
};
struct Point {
	double x, y;
};
struct Line {
	Point start, end;
};
struct Ball {
	struct Point pos;
	int r;
	struct Vec v;
	HBITMAP hbmp;
	int num;
	bool visible = true;
	bool enable = true;

	void render(HDC *bufdc, HDC * target_dc);
	bool isIntersect(const Line& ls);
	bool isIntersect(const Ball& other);
	void update(ULONGLONG time, double f);
	void hit_ball(Ball& other);
};


class Pool : public Game {
private:
	Log* log_ptr{ nullptr };
	Ball balls[16];
	HBITMAP table_bmp, triangle_bmp, cue_bmp;
	HDC g_hdc{ nullptr }, g_mdc{ nullptr }, g_bufdc{ nullptr };
	ULONGLONG g_tpre{ 0 }, g_tnow{ 0 };
	ULONGLONG r_tpre;
	HPEN g_pen{ nullptr };
	HWND hwnd_progress_bar;
	int progress = 0;
    Pos start_pos, end_pos;
    bool left_mouse_flag = false;
    bool right_mouse_flag = false;
public:
	void game_init(HWND& hwnd) override;
	void game_loop() override;
	void game_cleanup() override;
	std::wstring to_string() override;
	std::vector<std::wstring> game_models() override;
	void set_game_model(int idx) override;
	void game_process_key_down(HWND hwnd, UINT message, WPARAM wParam,
		LPARAM lParam) override;
	void render();
	void render_balls();
	void render_progress_bar();
	void update(ULONGLONG dt);
	RECT holes[6];
	RECT top_wall[2];
	RECT bottom_wall[2];
	RECT left_wall;
	RECT right_wall;
private:
	static constexpr int BALL_BMP_H{ 60 }, BALL_BMP_W{ 60 };
	static constexpr int WALL_WITDH{ 150}, WALL_HEIGHT{ 128 };
	void init_balls();
    void left_click_down(int width_pos, int height_pos);
    void left_click_up(int width_pos, int height_pos);
    void right_click_up(int width_pos, int height_pos);
    void right_click_down(int width_pos, int height_pos);
	void mouse_move(int width_pos, int height_pos);
	void cue_ball();
	void hit_wall(Ball & ball);
	bool hit_hole(Ball& ball);
	void init_holes();
	void init_walls();
};

bool point_in_rect(const Point& point, const RECT& rect);
bool rect_in_rect(const RECT& lhs, const RECT& rhs);

