#include "text_buffer.hpp"

#include "logger.hpp"

#include <cstring>
#include <cstdio>
#include <unistd.h>

TextBuffer::TextBuffer(const TextBufferSettings &settings) {
	fg = settings.fg;
	bg = settings.bg;
	tab_width = settings.tab_width;
	scrolloff = settings.scrolloff;
	strcpy(tab_char, settings.tab_char);
	strcpy(newline_char, settings.newline_char);

	number_column.x = settings.x;
	number_column.y = settings.y;
	number_column.width = 4;
	number_column.height = settings.height;

	text_area.x = settings.x + 4;
	text_area.y = settings.y;
	text_area.width = settings.width - 4;
	text_area.height = settings.height;
}

Result TextBuffer::loadBuffer(const std::string &filename) {
	if (gap_buffer.loadFile(filename)) return MEMORY_ERROR;

	screen_start_index = 0;
	screen_start_line = 1;

	if (text_area.init() != SUCCESS) return MEMORY_ERROR;
	if (number_column.init() != SUCCESS) return MEMORY_ERROR;
	updateFrame();

	return SUCCESS;
}

void TextBuffer::updateFrame() {
	for (unsigned int i = 0; i < number_column.height; i++) {
		Character ch = Character{CharColor::BLACK, CharColor::GREEN};
		unsigned int number_line = i + screen_start_line;
		for (unsigned int j = 4; j >= 1; j--) {
			ch.character[0] = number_line % 10 + '0';
			number_line /= 10;
			number_column.contents[number_column.width * i + j - 1] = ch;
		}
	}
	if (gap_buffer.length() == 0) {
		unsigned short screen_pos_x = 0, screen_pos_y = 0;
		while (screen_pos_y < text_area.height) {
			text_area.loadString(" ", 1, screen_pos_x, screen_pos_y, CharColor::GREEN, CharColor::BLACK);
			screen_pos_y++;
			screen_pos_x = 0;
		}
		text_area.draw();
		number_column.draw();
		return;
	}
	unsigned short screen_pos_x = 0, screen_pos_y = 0;
	size_t line_start = 0, line_length = 0;
	if (selection) {
		if (selection_start_index > gap_buffer.pre_cursor_index) {
			line_start = *(gap_buffer.pre_cursor_lines.begin() + screen_start_line - 1);
			line_length = gap_buffer.pre_cursor_index - line_start;
			text_area.loadString(&gap_buffer.buffer[line_start], line_length, screen_pos_x, screen_pos_y, CharColor::GREEN, CharColor::BLACK);
			line_start = gap_buffer.post_cursor_index;
			line_length = selection_start_index - gap_buffer.pre_cursor_index;
			text_area.loadString(&gap_buffer.buffer[line_start], line_length, screen_pos_x, screen_pos_y, CharColor::GREEN, CharColor::WHITE);
			if (screen_pos_y < text_area.height) {
				line_start = gap_buffer.post_cursor_index + line_length;
				line_length = gap_buffer.capacity - line_start;
				text_area.loadString(&gap_buffer.buffer[line_start], line_length, screen_pos_x, screen_pos_y, CharColor::GREEN, CharColor::BLACK);
			}
		} else {
			line_start = *(gap_buffer.pre_cursor_lines.begin() + screen_start_line - 1);
			if (line_start < selection_start_index) {
				line_length = selection_start_index - line_start;
				text_area.loadString(&gap_buffer.buffer[line_start], line_length, screen_pos_x, screen_pos_y, CharColor::GREEN, CharColor::BLACK);
				line_start = selection_start_index;
				line_length = gap_buffer.pre_cursor_index - selection_start_index;
				text_area.loadString(&gap_buffer.buffer[line_start], line_length, screen_pos_x, screen_pos_y, CharColor::GREEN, CharColor::WHITE);
			} else {
				line_length = gap_buffer.pre_cursor_index - line_start;
				text_area.loadString(&gap_buffer.buffer[line_start], line_length, screen_pos_x, screen_pos_y, CharColor::GREEN, CharColor::WHITE);
			}
			line_start = gap_buffer.post_cursor_index;
			line_length = gap_buffer.capacity - line_start;
			text_area.loadString(&gap_buffer.buffer[line_start], line_length, screen_pos_x, screen_pos_y, CharColor::GREEN, CharColor::BLACK);
		}
	} else {
		if (gap_buffer.pre_cursor_index > 0) {
			line_start = *(gap_buffer.pre_cursor_lines.begin() + screen_start_line - 1);
			line_length = gap_buffer.pre_cursor_index - line_start;
			text_area.loadString(&gap_buffer.buffer[line_start], line_length, screen_pos_x, screen_pos_y, CharColor::GREEN, CharColor::BLACK);
		}
		if (gap_buffer.post_cursor_index < gap_buffer.capacity) {
			line_start = gap_buffer.post_cursor_index;
			line_length = gap_buffer.capacity - line_start;
			text_area.loadString(&gap_buffer.buffer[line_start], line_length, screen_pos_x, screen_pos_y, CharColor::GREEN, CharColor::BLACK);
		}
	}
	while (screen_pos_y < text_area.height) {
		text_area.loadString(" ", 1, screen_pos_x, screen_pos_y, CharColor::GREEN, CharColor::BLACK);
		screen_pos_y++;
		screen_pos_x = 0;
	}
	
	text_area.draw();
	number_column.draw();
	getCursorPosition();
}

void TextBuffer::getCursorPosition() {
	printf("\033[%li;%liH", text_area.y + gap_buffer.pre_cursor_lines.size() - screen_start_line + 1, text_area.x + gap_buffer.get_line_index() + 1);
	fflush(stdout);
}

size_t TextBuffer::advance(size_t distance) {
	size_t result = gap_buffer.advance(distance);
	if (gap_buffer.pre_cursor_lines.size() > text_area.height * 3 / 4 + screen_start_line) {
		screen_start_line = gap_buffer.pre_cursor_lines.size() - text_area.height * 3 / 4;
		updateFrame();
	} else if (selection) {
		updateFrame();
	} else {
		getCursorPosition();
	}
	return result;
}

size_t TextBuffer::end() {
	size_t result = gap_buffer.end();
	if (selection) updateFrame();
	else getCursorPosition();
	return result;
}

size_t TextBuffer::down(size_t distance) {
	size_t result = gap_buffer.down(distance);
	if (gap_buffer.pre_cursor_lines.size() > text_area.height * 3 / 4 + screen_start_line) {
		screen_start_line = gap_buffer.pre_cursor_lines.size() - text_area.height * 3 / 4;
		updateFrame();
	} else if (selection) {
		updateFrame();
	} else {
		getCursorPosition();
	}
	return result;
}

size_t TextBuffer::retreat(size_t distance) {
	size_t result = gap_buffer.retreat(distance);
	bool redraw = false;
	if (screen_start_line == 1) {
		redraw = false;
	} else if (gap_buffer.pre_cursor_lines.size() < text_area.height / 4) {
		screen_start_line = 1;
		redraw = true;
	} else if (gap_buffer.pre_cursor_lines.size() < text_area.height / 4 + screen_start_line) {
		screen_start_line = gap_buffer.pre_cursor_lines.size() - text_area.height / 4;
		redraw = true;
	}
	if (selection) redraw = true;
	if (redraw) updateFrame();
	else getCursorPosition();
	return result;
}

size_t TextBuffer::home() {
	size_t result = gap_buffer.home();
	if (selection) updateFrame();
	else getCursorPosition();
	return result;
}

size_t TextBuffer::up(size_t distance) {
	size_t result = gap_buffer.up(distance);
	bool redraw = false;
	if (screen_start_line == 1) {
		redraw = false;
	} else if (gap_buffer.pre_cursor_lines.size() < text_area.height / 4) {
		screen_start_line = 1;
		redraw = true;
	} else if (gap_buffer.pre_cursor_lines.size() < text_area.height / 4 + screen_start_line) {
		screen_start_line = gap_buffer.pre_cursor_lines.size() - text_area.height / 4;
		redraw = true;
	}
	if (selection) redraw = true;
	if (redraw) updateFrame();
	else getCursorPosition();
	return result;
}

size_t TextBuffer::move(size_t line) {
	size_t result = 0;
	if (line > gap_buffer.pre_cursor_lines.size()) {
		result = down(line - gap_buffer.pre_cursor_lines.size());
	} else {
		result = up(gap_buffer.pre_cursor_lines.size() - line);
	}
	return result;
}

size_t TextBuffer::insert(const char *data, size_t length) {
	size_t insert_count = gap_buffer.insert(data, length);
	/*
	if (length == 1) {
		switch (data[0]) {
			case '}':
			case ']':
				
				break;
		}
	}*/
	if (gap_buffer.pre_cursor_lines.size() > text_area.height * 3 / 4 + screen_start_line)
		screen_start_line += gap_buffer.pre_cursor_lines.size() - text_area.height * 3 / 4 - screen_start_line;
	updateFrame();
	return insert_count;
}

size_t TextBuffer::removeFront(size_t length) {
	size_t remove_count = gap_buffer.removeFront(length);
	updateFrame();
	return remove_count;
}

size_t TextBuffer::removeBack(size_t length) {
	size_t remove_count = gap_buffer.removeBack(length);
	updateFrame();
	return remove_count;
}

void TextBuffer::saveFile(FILE *file) {
	gap_buffer.print(file);
}

void TextBuffer::beginSelection() {
	selection_start_index = gap_buffer.pre_cursor_index;
	selection = true;
}

void TextBuffer::cancelSelection() {
	selection_start_index = 0;
	selection = false;
	updateFrame();
}

void TextBuffer::deleteSelection() {
	if (selection_start_index > gap_buffer.pre_cursor_index) {
		gap_buffer.removeFront(selection_start_index - gap_buffer.pre_cursor_index);
	} else if (selection_start_index < gap_buffer.pre_cursor_index) {
		gap_buffer.removeBack(gap_buffer.pre_cursor_index - selection_start_index);
	}
}

const char base_64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

size_t toBase64(const char *bytes, size_t length, char *base_64) {
	size_t index = 0;
	size_t i = 0;
	if (length > 3069) {
		length = 3069;
	}
	for (; i < length; i++) {
		if (index % 4 == 0) {
			base_64[index] = base_64_chars[(bytes[i] & 0b11111100) >> 2];
			index++;
			base_64[index] = base_64_chars[((bytes[i] & 0b00000011) << 4) + ((bytes[i + 1] & 0b11110000) >> 4)];
			index++;
		} else if (index % 4 == 2) {
			base_64[index] = base_64_chars[((bytes[i] & 0b00001111) << 2) + ((bytes[i + 1] & 0b11000000) >> 6)];
			index++;
		} else if (index % 4 == 3) {
			base_64[index] = base_64_chars[(bytes[i] & 0b00111111)];
			index++;
		}
	}
	base_64[index] = '\0';
	return i;
}

void TextBuffer::getSelection() {
	size_t buffer_length = 0;
	char base_64[4096] = {0};
	printf("\033]52;c;");
	char *buffer;
	if (selection_start_index >= gap_buffer.pre_cursor_index) {
		buffer_length = selection_start_index - gap_buffer.pre_cursor_index - 1;
		buffer = &gap_buffer.buffer[gap_buffer.post_cursor_index];
	} else {
		buffer_length = gap_buffer.pre_cursor_index - selection_start_index;
		buffer = &gap_buffer.buffer[selection_start_index];
	} 
	while (buffer_length > 0) {
		size_t length = toBase64(buffer, buffer_length, base_64);
		buffer_length -= length;
		buffer += length;
		fwrite(base_64, 1, strlen(base_64), stdout);
	}
	printf("\033\\");
	fflush(stdout);
}

size_t TextBuffer::scopeCount() {
	size_t scope_count = 0;
	size_t open_brace = 0;
	for (size_t i = gap_buffer.pre_cursor_lines.back(); i < gap_buffer.pre_cursor_index; i++) {
		switch (gap_buffer.buffer[i]) {
			case '\t':
				scope_count++;
				break;
			case '[':
			case '{':
			case ':':
				open_brace = 1;
				break;
			case '}':
			case ']':
				open_brace = 0;
				break;
		}
	}
	return scope_count + open_brace;
}