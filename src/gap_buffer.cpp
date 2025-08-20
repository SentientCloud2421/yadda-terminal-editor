#include "gap_buffer.hpp"

#include "logger.hpp"

#include <cstdio>
#include <cstring>

constexpr unsigned BLOCK_SIZE = 4096;

GapBuffer::~GapBuffer() {
	if (buffer) {
		delete[] buffer;
	}
}

Result GapBuffer::loadFile(const std::string &filename) {
	assert(!buffer, NULL_ERROR, "buffer must be null!");
	assert(filename != "", IO_ERROR, "filename must not be empty!");
	
	FILE *file = fopen(filename.c_str(), "r");
	if (!file) {
		Logger::error("failed to open file!");
		return IO_ERROR;
	}
	fseek(file, 0, SEEK_END);
	size_t len = (size_t)ftell(file);
	fseek(file, 0, SEEK_SET);
	
	if (buffer) {
		if (capacity < (len / BLOCK_SIZE + 1) * BLOCK_SIZE) {
			delete[] buffer;
			capacity = (len / BLOCK_SIZE + 1) * BLOCK_SIZE;
			buffer = new char[capacity];
		}
		pre_cursor_lines.clear();
		post_cursor_lines.clear();
	} else {
		capacity = (len / BLOCK_SIZE + 1) * BLOCK_SIZE;
		buffer = new char[capacity];
	}
	pre_cursor_index = fread(buffer, 1, len, file);
	fclose(file);
	
	post_cursor_index = capacity;
	bool line_start = false;
	pre_cursor_lines.push_back(0);
	for (size_t i = 0; i < pre_cursor_index; i++) {
		if (line_start) {
			pre_cursor_lines.push_back(i);
			line_start = false;
		}
		if (buffer[i] == '\n') {
			line_start = true;
		}
	}
	retreat(pre_cursor_index);
	line_index = 0;
	
	return SUCCESS;
}

/*
 * Inserts characters at the current cursor position and
 * advances the cursor. Automatically handles inserting
 * line starts.
 */
size_t GapBuffer::insert(const char *data, size_t length) {
	assert(buffer, 0, "buffer must be allocated!");
	assert(data, 0, "data must be non-null!");
	assert(pre_cursor_index < post_cursor_index, 0, "pre_cursor_index must be less than post_cursor_index!");
	
	if (length + pre_cursor_index >= post_cursor_index) {
		resize(capacity + BLOCK_SIZE);
	}
	
	size_t i = 0;
	bool line_start = false;
	for (; i < length; i++) {
		buffer[pre_cursor_index] = data[i];
		if (data[i] == '\t') {
			line_index = (line_index / 4 + 1) * 4;
		} else if ((data[i] & 0xC0) != 0x80) {
			line_index++;
		}
		if (line_start) {
			pre_cursor_lines.push_back(pre_cursor_index);
			line_start = false;
			line_index = 0;
		}
		if (data[i] == '\n') {
			line_start = true;
		}
		pre_cursor_index++;
	}
	if (line_start) {
		pre_cursor_lines.push_back(pre_cursor_index);
		line_index = 0;
	}
	
	return i;
}

size_t GapBuffer::removeFront(size_t length) {
	assert(buffer, 0, "buffer must be allocated!");
	assert(post_cursor_index <= capacity, 0, "post_cursor_index must be less than or equal to capacity");
	if (post_cursor_index + length >= capacity) {
		Logger::error("attempting to remove at the end of buffer!");
		length = capacity - post_cursor_index;
	}
	
	size_t i = 0;
	for (; i < length; i++) {
		if (!post_cursor_lines.empty()) {
			if (capacity - post_cursor_index == post_cursor_lines.back()) {
				post_cursor_lines.pop_back();
			}
		}
		post_cursor_index++;
	}
	
	return i;
}

/*
 * Removes characters behind the current cursor position and
 * decrements the cursor. Automatically handles removing line-
 * starts and calculates line_index.
 */
size_t GapBuffer::removeBack(size_t length) {
	assert(buffer, 0, "buffer must be allocated!");
	assert(pre_cursor_index < post_cursor_index, 0, "pre_cursor_index must be less than post_cursor_index");
	if (pre_cursor_index < length) {
		Logger::error("attempting to remove at the beginning of buffer!");
		length = pre_cursor_index;
	}
	
	size_t i = 0;
	bool recalc_line_index = false;
	for (; i < length; i++) {
		if (buffer[pre_cursor_index - 1] == '\t') {
			recalc_line_index = true;
		} else if ((buffer[pre_cursor_index - 1] & 0xC0) != 0x80) {
			line_index--;
		}
		if (pre_cursor_index == pre_cursor_lines.back()) {
			pre_cursor_lines.pop_back();
			recalc_line_index = true;
		}
		pre_cursor_index--;
	}
	if (recalc_line_index) {
		line_index = get_line_index();
	}
	
	return i;
}

size_t GapBuffer::resize(size_t new_capacity) {
	assert(buffer, 0, "buffer must be allocated!");
	assert(new_capacity > capacity, 0, "new_capacity must be greater than current capacity!");
	
	size_t new_pci = new_capacity - (capacity - post_cursor_index);
	char *new_buffer = new char[new_capacity];
	memcpy(new_buffer, buffer, capacity);
	memmove(&new_buffer[new_pci], &new_buffer[post_cursor_index], new_capacity - new_pci);
	post_cursor_index = new_pci;
	capacity = new_capacity;
	delete[] buffer;
	buffer = new_buffer;
	
	return new_capacity;
}

size_t GapBuffer::advance(size_t distance) {
	assert(buffer, 0, "buffer must be allocated!");
	assert(pre_cursor_index < post_cursor_index, 0, "pre_cursor_index must be less than post_cursor_index!");
	assert(post_cursor_index <= capacity, 0, "post_cursor_index cannot be greater than capacity!");
	if (distance + post_cursor_index > capacity) {
		Logger::error("attempting to advance past buffer!");
		distance = capacity - post_cursor_index;
	}
	size_t i = 0;
	for (; i < distance; i++) {
		buffer[pre_cursor_index] = buffer[post_cursor_index];
		pre_cursor_index++;
		post_cursor_index++;
		if (post_cursor_lines.size() > 0) {
			if (post_cursor_lines.back() == capacity - post_cursor_index) {
				pre_cursor_lines.push_back(pre_cursor_index);
				post_cursor_lines.pop_back();
			}
		}
	}
	line_index = get_line_index();
	return i;
}

size_t GapBuffer::retreat(size_t distance) {
	assert(buffer, 0, "buffer must be allocated!");
	assert(pre_cursor_index < post_cursor_index, 0, "pre_cursor_index must be less than post_cursor_index!");
	assert(post_cursor_index <= capacity, 0, "post_cursor_index cannot be greater than capacity!");
	
	if (distance > pre_cursor_index) {
		Logger::error("attempting to retreat past buffer!");
		distance = pre_cursor_index;
	}
	size_t i = 0;
	for (; i < distance; i++) {
		if (pre_cursor_lines.size() > 1) {
			if (pre_cursor_lines.back() == pre_cursor_index) {
				post_cursor_lines.push_back(capacity - post_cursor_index);
				pre_cursor_lines.pop_back();
			}
		}
		pre_cursor_index--;
		post_cursor_index--;
		buffer[post_cursor_index] = buffer[pre_cursor_index];
	}
	line_index = get_line_index();
	return i;
}

size_t GapBuffer::end() {
	assert(buffer, 0, "buffer must be allocated!");
	assert(pre_cursor_index < post_cursor_index, 0, "pre_cursor_index must be less than post_cursor_index!");
	assert(post_cursor_index <= capacity, 0, "post_cursor_index cannot be greater than capacity!");
	if (post_cursor_lines.empty()) {
		return advance(capacity - post_cursor_index);
	} else {
		return advance(capacity - post_cursor_index - post_cursor_lines.back() - 1);
	}
}

size_t GapBuffer::home() {
	assert(buffer, 0, "buffer must be allocated!");
	assert(pre_cursor_index < post_cursor_index, 0, "pre_cursor_index must be less than post_cursor_index!");
	assert(post_cursor_index <= capacity, 0, "post_cursor_index cannot be greater than capacity!");
	return retreat(pre_cursor_index - pre_cursor_lines.back());
}

size_t GapBuffer::up(size_t distance) {
	assert(buffer, 0, "buffer must be allocated!");
	assert(pre_cursor_index < post_cursor_index, 0, "pre_cursor_index must be less than post_cursor_index!");
	assert(post_cursor_index <= capacity, 0, "post_cursor_index cannot be greater than capacity!");
	
	size_t temp_line_index = line_index;
	if (pre_cursor_lines.size() == 1) {
		return 0;
	}
	if (distance >= pre_cursor_lines.size()) {
		Logger::error("attempting to move out of bounds!");
		distance = pre_cursor_lines.size() - 1;
	}
	size_t line_length = *(pre_cursor_lines.end() - distance) - *(pre_cursor_lines.end() - distance - 1) - 1;
	size_t cursor_pos_x = 0;
	for (size_t i = *(pre_cursor_lines.end() - distance - 1); i < *(pre_cursor_lines.end() - distance); i++) {
		if (buffer[i] == '\t') {
			cursor_pos_x = (cursor_pos_x / 4 + 1) * 4;
		} else if (buffer[i] == '\n') {
			break;
		} else if ((buffer[i] & 0xC0) != 0x80) {
			cursor_pos_x++;
		}
		if (temp_line_index <= cursor_pos_x) {
			line_length = i - *(pre_cursor_lines.end() - distance - 1) + (1 && temp_line_index);
			break;
		}
	}
	size_t dist = pre_cursor_index - *(pre_cursor_lines.end() - distance - 1) - line_length;
	retreat(dist);
	line_index = temp_line_index;
	return distance;
}

size_t GapBuffer::down(size_t distance) {
	assert(buffer, 0, "buffer must be allocated!");
	assert(pre_cursor_index < post_cursor_index, 0, "pre_cursor_index must be less than post_cursor_index!");
	assert(post_cursor_index <= capacity, 0, "post_cursor_index cannot be greater than capacity!");
	
	size_t temp_line_index = line_index;
	if (post_cursor_lines.size() == 0) {
		return 0;
	}
	if (distance > post_cursor_lines.size()) {
		Logger::error("attempting to move out of bounds!");
		distance = post_cursor_lines.size();
	}
	size_t line_length = 0;
	if (post_cursor_lines.size() == distance) {
		line_length = *(post_cursor_lines.end() - distance);
	} else {
		line_length = *(post_cursor_lines.end() - distance) - *(post_cursor_lines.end() - distance - 1) - 1;
	}
	size_t cursor_pos_x = 0;
	for (size_t i = capacity - *(post_cursor_lines.end() - distance); i < capacity; i++) {
		if (buffer[i] == '\t') {
			cursor_pos_x = (cursor_pos_x / 4 + 1) * 4;
		} else if (buffer[i] == '\n') {
			break;
		} else if ((buffer[i] & 0xC0) != 0x80) {
			cursor_pos_x++;
		}
		if (temp_line_index <= cursor_pos_x) {
			line_length = i - (capacity - *(post_cursor_lines.end() - distance)) + (1 && temp_line_index);
			break;
		}
	}
	char buf[256] = {0};
	sprintf(buf, "line_length: %lu", line_length);
	Logger::info(buf);
	size_t dist = capacity - post_cursor_index - *(post_cursor_lines.end() - distance) + line_length;
	advance(dist);
	line_index = temp_line_index;
	return distance;
}

size_t GapBuffer::length() {
	assert(buffer, 0, "buffer must be allocated!");
	return capacity - post_cursor_index + pre_cursor_index;
}

size_t GapBuffer::get_line_index() {
	assert(buffer, 0, "buffer must be allocated!");
	size_t index = 0;
	for (size_t i = pre_cursor_lines.back(); i < pre_cursor_index; i++) {
		if (buffer[i] == '\t') {
			index = (index / 4 + 1) * 4;
		} else if ((buffer[i] & 0xC0) != 0x80) { // ignores trailing unicode
			index++;
		}
	}
	return index;
}

Result GapBuffer::print(FILE *file) {
	assert(file, IO_ERROR, "file must be valid open file or stdout!");
	assert(buffer, NULL_ERROR, "buffer must be allocated!");
	assert(pre_cursor_index < post_cursor_index, OUT_OF_BOUNDS, "pre_cursor_index must be less than post_cursor_index!");
	assert(post_cursor_index <= capacity, OUT_OF_BOUNDS, "post_cursor_index cannot be greater than capacity!");
	if (pre_cursor_index > 0) {
		fwrite(buffer, 1, pre_cursor_index, file);
	}
	if (file == stdout) {
		printf("^");
	}
	if (capacity > post_cursor_index) {
		fwrite(&buffer[post_cursor_index], 1, capacity - post_cursor_index, file);
	}
	if (file == stdout) {
		printf("\n");
	} else {
		fflush(file);
	}
	
	return SUCCESS;
}