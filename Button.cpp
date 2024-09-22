#include "button.hpp"


std::string Button::to_string() {
	char buf[1024];
	sprintf_s(buf, "id:%d\thwnd:%p", this->id, this->hwnd);
	std::string ret(buf);
	return ret;
}
