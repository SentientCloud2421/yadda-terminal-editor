#pragma once

#include "defines.hpp"

#include <cstdio>
#include <vector>
#include <string>

/* GapBuffer
 * loadFile: takes a filename and loads the contents of the
 *   file into the buffer.
 * insert: inserts the supplied text, up to length bytes, and
 *   advances the cursor.
 * removeFront: removes data from the front of the cursor. (delete)
 * removeBack: removes data from the back of the cursor. (backspace)
 * advance: moves the cursor forward in the buffer.
 * retreat: moves the cursor backward in the buffer.
 * end: moves to the end of the current line.
 * home: moves to the beginning of the current line.
 * up: moves up to distance lines up.
 * down: moves up to distance lines down.
 * get_line_index: calculates the current line index.
 * print: sends the contents of the buffer to a file, or the
 *   terminal.
 * resize: increases the size of the buffer and copies the data over.
 * buffer: stores all of the text data for the buffer.
 * capacity: unsigned int that does what it says on the tin.
 *   Includes the space between the two sides of the gap.
 * pre_cursor_index: stores the top of the pre-cursor data, and 
 *   is set to be one position ahead. (makes the math easier).
 * post_cursor_index: stores the bottom of the post-cursor
 *   data, and is set to be on top of the last byte.
 * pre_cursor_lines: stores all of the line start indices before
 *   the cursor. Will always have 0 as its first element.
 * post_cursor_lines: stores all of the line start indices after
 *   the cursor. Can be empty, if the cursor is on the last newline.
 * line_index: preserves the position of the cursor along the line.
 *   Respects tabwidth, and unicode.
 */

struct GapBuffer {
	~GapBuffer();
	
	Result loadFile(const std::string &filename);
	size_t insert(const char *data, size_t length);
	size_t removeFront(size_t length);
	size_t removeBack(size_t length);
	size_t advance(size_t distance);
	size_t retreat(size_t distance);
	size_t end();
	size_t home();
	size_t up(size_t distance);
	size_t down(size_t distance);
	size_t length();
	size_t get_line_index();
	Result print(FILE *file = stdout);

	char *buffer = nullptr;
	size_t capacity = 0;
	size_t pre_cursor_index = 0;
	size_t post_cursor_index = 0;
	std::vector<size_t> pre_cursor_lines;
	std::vector<size_t> post_cursor_lines;
	size_t line_index;

private:
	size_t resize(size_t new_capacity);
};
