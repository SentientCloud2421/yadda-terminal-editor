#include "screen.hpp"

#include "logger.hpp"

#include <cstring>
#include <sstream>

using byte = unsigned char;

Result Frame::init() {
	if (contents != nullptr) delete[] contents;
	contents = new (std::nothrow) Character[width * height];
	if (contents == nullptr) return MEMORY_ERROR;
	return SUCCESS;
}

void Frame::loadString(const char *string, size_t length, unsigned short &x, unsigned short &y, CharColor fg, CharColor bg) {
	if (x >= width || y >= height || length == 0) {
		return;
	}
	size_t i = 0;
	bool new_line = false;
	unsigned short temp_x = 0;
	unsigned short temp_y = 0;
	for (; y < height; y++) {
		for (; x < width; x++) {
			if (i < length && !new_line) {
				if (string[i] == '\t') {
					contents[width * y + x] = Character{fg, bg, ">"};
					x++;
					for (; x % 4 != 0; x++) {
						contents[width * y + x] = Character{fg, bg, " "};
					}
					x--;
				} else if (string[i] == '\n') {
					contents[width * y + x] = Character{fg, bg, " "};
					new_line = true;
					temp_x = 0;
					temp_y = y + 1;
					i++;
					continue;
				} else if ((byte)string[i] < 0x80) {
					contents[width * y + x] = Character{fg, bg, {string[i]}};
				} else if ((byte)string[i] < 0b11100000) {
					Character character{fg, bg, {string[i++]}};
					strncpy(&character.character[1], &string[i], 1);
					contents[width * y + x] = character;
				} else if ((byte)string[i] < 0b11110000) {
					Character character{fg, bg, {string[i++]}};
					strncpy(&character.character[1], &string[i], 2);
					contents[width * y + x] = character;
					i += 1;
				} else if ((byte)string[i] < 0b11111000) {
					Character character{fg, bg, {string[i++]}};
					strncpy(&character.character[1], &string[i], 3);
					contents[width * y + x] = character;
					i += 2;
				} else if ((byte)string[i] < 0b11111100) {
					Character character{fg, bg, {string[i++]}};
					strncpy(&character.character[1], &string[i], 4);
					contents[width * y + x] = character;
					i += 3;
				}
				i++;
				temp_x = x + 1;
				temp_y = y;
			} else {
				contents[width * y + x] = Character{fg, bg, " "};
			}
		}
		for (; !new_line && i < length; i++) {
			if (string[i] == '\n') {
				new_line = true;
				temp_x = 0;
				temp_y = y + 1;
			}
		}
		new_line = false;
		x = 0;
	}
	y = temp_y;
	x = temp_x;
}

void Frame::draw() {
	CharColor current_fg = contents[0].fg;
	CharColor current_bg = contents[0].bg;

	std::stringstream print_string;
	print_string << "\033[" << parseFgColor(current_fg) << ';' << parseBgColor(current_bg) << 'm';
	for (unsigned int i = 0; i < width * height; i++) {
		if (i % width == 0) {
			print_string << "\033[" << y + i / width + 1 << ';' << x + 1 << 'H';
		}
		print_string << parse(contents[i], current_fg, current_bg);
		print_string << contents[i].character;
	}
	printf("%s", print_string.str().c_str());
	fflush(stdout);
}

std::string parse(Character &c, CharColor &current_fg, CharColor &current_bg) {
	std::string print_string = "";
	if (current_fg != c.fg ||
		current_bg != c.bg) {
		bool sequence_separator = false;
		print_string += "\033[";
		if (current_fg != c.fg) {
			print_string += parseFgColor(c.fg);
			current_fg = c.fg;
			sequence_separator = true;
		}
		if (current_bg != c.bg) {
			if (sequence_separator) print_string += ';';
			print_string += parseBgColor(c.bg);
			current_bg = c.bg;
			sequence_separator = true;
		}
		print_string += 'm';
	}
	return print_string;
}

std::string parseFgColor(CharColor fg) {
	switch (fg) {
		case CharColor::BLACK: return "38;2;16;12;8";
		case CharColor::RED: return "31";
		case CharColor::GREEN: return "38;2;159;95;0";
		case CharColor::YELLOW: return "33";
		case CharColor::BLUE: return "34";
		case CharColor::MAGENTA: return "35";
		case CharColor::CYAN: return "36";
		case CharColor::WHITE: return "38;2;48;32;8";
		case CharColor::LIGHT_BLACK: return "90";
		case CharColor::LIGHT_RED: return "91";
		case CharColor::LIGHT_GREEN: return "92";
		case CharColor::LIGHT_YELLOW: return "93";
		case CharColor::LIGHT_BLUE: return "94";
		case CharColor::LIGHT_MAGENTA: return "95";
		case CharColor::LIGHT_CYAN: return "96";
		case CharColor::LIGHT_WHITE: return "97";
		default: return "";
	}
}

std::string parseBgColor(CharColor bg) {
	switch (bg) {
		case CharColor::BLACK: return "48;2;16;12;8";
		case CharColor::RED: return "41";
		case CharColor::GREEN: return "48;2;159;95;0";
		case CharColor::YELLOW: return "43";
		case CharColor::BLUE: return "44";
		case CharColor::MAGENTA: return "45";
		case CharColor::CYAN: return "46";
		case CharColor::WHITE: return "48;2;48;32;8";
		case CharColor::LIGHT_BLACK: return "100";
		case CharColor::LIGHT_RED: return "101";
		case CharColor::LIGHT_GREEN: return "102";
		case CharColor::LIGHT_YELLOW: return "103";
		case CharColor::LIGHT_BLUE: return "104";
		case CharColor::LIGHT_MAGENTA: return "105";
		case CharColor::LIGHT_CYAN: return "106";
		case CharColor::LIGHT_WHITE: return "107";
		default: return "";
	}
}
