#pragma once

#include "defines.hpp"
#include "screen.hpp"
#include "gap_buffer.hpp"

#include <cstdio>
#include <string>

struct TextBufferSettings {
	unsigned int x = 0, y = 0;
	unsigned int width = 80;
	unsigned int height = 40;
	CharColor fg = CharColor::WHITE;
	CharColor bg = CharColor::BLACK;
	unsigned int scrolloff = 8;
	unsigned int tab_width = 4;
	char tab_char[4] = " ";
	char newline_char[4] = " ";
};

class TextBuffer {
public:
	TextBuffer(const TextBufferSettings &settings);

	Result loadBuffer(const std::string &filename);
	size_t getBufferSize() { return gap_buffer.length(); }
	void getCursorPosition();

	size_t shiftUp();
	size_t shiftDown();
	size_t advance(size_t distance);
	size_t down(size_t distance);
	size_t end();
	size_t retreat(size_t distance);
	size_t home();
	size_t up(size_t distance);
	size_t move(size_t line);
	size_t insert(const char *data, size_t length);
	size_t removeFront(size_t length);
	size_t removeBack(size_t length);
	long getCursorX() { return gap_buffer.line_index; }
	void saveFile(FILE *file);
	
	void beginSelection();
	void cancelSelection();
	void deleteSelection();
	void getSelection();
	size_t scopeCount();
	
private:
	void getChar(char buffer[5], unsigned int &i);
	void updateFrame();
	Result resizeBuffer(long length);
	// settings
	unsigned int tab_width;
	unsigned int scrolloff;
	CharColor bg = CharColor::BLACK;
	CharColor fg = CharColor::WHITE;
	char tab_char[4] = " ";
	char newline_char[4] = " ";

	// screen member variables
	size_t screen_start_index = 0;
	size_t screen_end_index = 0;
	size_t screen_start_line = 1;
	size_t cursor_y = 0;
	Frame text_area;
	Frame number_column;
	
	GapBuffer gap_buffer;
	
	// selection
	size_t selection_start_index = 0;
	bool selection = false;
}; 