#pragma once

#include "defines.hpp"

#include <string>

enum class CharColor {
	BLACK = 0,
	RED,
	GREEN,
	YELLOW,
	BLUE,
	MAGENTA,
	CYAN,
	WHITE,
	LIGHT_BLACK,
	LIGHT_RED,
	LIGHT_GREEN,
	LIGHT_YELLOW,
	LIGHT_BLUE,
	LIGHT_MAGENTA,
	LIGHT_CYAN,
	LIGHT_WHITE,
};

struct Character {
	CharColor fg = CharColor::WHITE;
	CharColor bg = CharColor::BLACK;
	char character[8] = " ";
};

struct Frame {
	~Frame() { if (contents != nullptr) delete[] contents; }
	Result init();
	void loadString(const char *string, size_t length, unsigned short &x, unsigned short &y, CharColor fg, CharColor bg);
	void draw();

	Character *contents = nullptr;
	unsigned int x = 0, y = 0;
	unsigned int width = 1, height = 1;
};

std::string parse(Character &c, CharColor &fg, CharColor &bg);
std::string parseFgColor(CharColor fg);
std::string parseBgColor(CharColor bg);