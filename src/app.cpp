#include "app.hpp"

#include "logger.hpp"

#include <cstdio>
#include <cctype>
#include <cstring>
#include <unistd.h>
#include <sys/ioctl.h>

const char *MODE_STRINGS[] = {
	" NORMAL ",
	" INSERT ",
	" SELECT ",
	" REPLACE ",
	" COMMAND "
};

const CharColor MODE_COLORS[] = {
	CharColor::GREEN,
	CharColor::GREEN,
	CharColor::GREEN,
	CharColor::GREEN,
	CharColor::GREEN,
};

Application::~Application() {
	write(STDOUT_FILENO, "\033[?1049l", sizeof("\033[?1049l") - 1);
	tcsetattr(STDIN_FILENO, TCSANOW, &terminal_settings);
	Logger::deinit();
	if (text_buffer != nullptr) delete text_buffer;
}

Result Application::init(const char *filename) {
	Result result = SUCCESS;
	TextBufferSettings settings;
	winsize w;
	ioctl(0, TIOCGWINSZ, &w);

	tcgetattr(STDIN_FILENO, &terminal_settings);
	termios new_settings = terminal_settings;
	cfmakeraw(&new_settings);
	new_settings.c_cc[VMIN] = 0;
	new_settings.c_cc[VTIME] = 1;
	tcsetattr(STDIN_FILENO, TCSANOW, &new_settings);

	settings.x = 0;
	settings.y = 0;
	settings.width = w.ws_col;
	settings.height = w.ws_row - 2;
	settings.fg = CharColor::WHITE;
	settings.bg = CharColor::BLACK;
	settings.tab_width = 4;
	settings.tab_char[0] = '\xEF';
	settings.tab_char[1] = '\x84';
	settings.tab_char[2] = '\x81';

	text_buffer = new TextBuffer(settings);
	if (text_buffer == nullptr) {
		Logger::fatal("Failed to allocate memory! Aborting...");
		return MEMORY_ERROR;
	}

	printf("\033[?1049h");

	modeline.x = 0;
	modeline.y = w.ws_row - 2;
	modeline.width = w.ws_col;
	modeline.height = 1;
	modeline.init();
	updateModeline();

	command_line.x = 0;
	command_line.y = w.ws_row - 1;
	command_line.width = w.ws_col;
	command_line.height = 1;
	command_line.init();
	command_line.draw();

	if (filename != nullptr) {
		debug("File opened: ", filename);
		this->filename = filename;
		result = text_buffer->loadBuffer(filename);
		if (result != SUCCESS) {
			return MEMORY_ERROR;
		}
		updateModeline();
	}
	
	return SUCCESS;
}

void Application::run() {
	running = true;
	while (running) {
		processInput();
	}
}

void Application::updateModeline() {
	std::string mode_string = MODE_STRINGS[static_cast<int>(mode)];
	std::string filename_string = " " + filename + " ";
	for (unsigned long i = 0; i < mode_string.length(); i++) {
		modeline.contents[i] = Character{CharColor::BLACK, MODE_COLORS[static_cast<int>(mode)], {mode_string[i]}};
	}
	for (unsigned long i = 0; i < filename_string.length(); i++) {
		modeline.contents[i + mode_string.length()] = Character{MODE_COLORS[static_cast<int>(mode)], CharColor::BLACK, {filename_string[i]}};
	}
	unsigned int modified_length = 0;
	if (modified) {
		modified_length = 3;
		char mod_string[] = "[+]";
		for (unsigned long i = 0; i < 3; i++) {
			modeline.contents[i + mode_string.length() + filename_string.length()] =
				Character{MODE_COLORS[static_cast<int>(mode)], CharColor::BLACK, {mod_string[i]}};
		}
	}
	Character c;
	for (unsigned int i = mode_string.length() + filename_string.length() // comment
		 + modified_length; i < modeline.width - 1; i++) {
		modeline.contents[i] = c;
	}
	modeline.draw();
}

void Application::processInput() {
	text_buffer->getCursorPosition();
	long length = read(STDIN_FILENO, input, 4095);
	input[length] = '\0';
	if (length == 0) {
		return;
	}
	bool handled;
	switch (mode) {
		case Mode::NORMAL: handled = processNormalInput(); break;
		case Mode::INSERT: handled = processInsertInput(); break;
		case Mode::SELECT: handled = processSelectInput(); break;
		case Mode::REPLACE: handled = processReplaceInput(); break;
		case Mode::COMMAND: handled = processCommandInput(); break;
	}
	if (handled == false) {
		processGlobalInput();
	}
}

const char base_64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void toBytes(const char *base_64, char *bytes) {
	int index = 0;
	for (int i = 0; base_64[i] != '\0' && base_64[i] != '=' && base_64[i] != '\\'; i++) {
		char byte = 0;
		for (; (unsigned long)byte < sizeof(base_64_chars) - 1; byte++) {
			if (base_64_chars[(int)byte] == base_64[i]) {
				break;
			}
		}
		if (i % 4 == 0) {
			bytes[index] = (byte << 2) & 0b11111100;
		} else if (i % 4 == 1) {
			bytes[index] |= (byte >> 4) & 0b00000011;
			// debug(bytes[index]);
			index++;
			bytes[index] = (byte << 4) & 0b11110000;
		} else if (i % 4 == 2) {
			bytes[index] |= (byte >> 2) & 0b00001111;
			// debug(bytes[index]);
			index++;
			bytes[index] = (byte << 6) & 0b11000000;
		} else if (i % 4 == 3) {
			bytes[index] |= byte & 0b00111111;
			// debug(bytes[index]);
			index++;
		}
	}
	bytes[index] = '\0';
}

bool Application::processNormalInput() {
	switch (input[0]) {
		case 'j': {
			if (command_number == "") {
				text_buffer->down(1);
			} else {
				text_buffer->down(std::stoul(command_number, nullptr));
				command_number = "";
			}
		} break;
		case 'k': {
			if (command_number == "") {
				text_buffer->up(1);
			} else {
				text_buffer->up(std::stoul(command_number, nullptr));
				command_number = "";
			}
		} break;
		case 'h': {
			if (command_number.empty()) {
				text_buffer->retreat(1);
			} else {
				text_buffer->retreat(std::stoul(command_number, nullptr));
				command_number = "";
			}
		} break;
		case 'l': {
			if (command_number.empty()) {
				text_buffer->advance(1);
			} else {
				text_buffer->advance(std::stoul(command_number, nullptr));
				command_number = "";
			}
		} break;
		case 'y': {
			mode = Mode::SELECT;
			updateModeline();
			command = "y";
			text_buffer->beginSelection();
		} break;
		case 'p': {
			printf("\033]52;c;?\033\\");
			fflush(stdout);
		} break;
		case 'a':
			text_buffer->advance(1);
			[[fallthrough]];
		case 'i': {
			mode = Mode::INSERT;
			updateModeline();
			printf("\033[5 q");
			fflush(stdout);
		} break;
		case 'd': {
			mode = Mode::SELECT;
			updateModeline();
			command = "d";
			text_buffer->beginSelection();
		} break;
		case 'r': {
			mode = Mode::REPLACE;
			updateModeline();
			printf("\033[3 q");
			fflush(stdout);
		} break;
		case 'm': {
			if (!command_number.empty()) {
				text_buffer->move(std::stoul(command_number, nullptr));
				command_number = "";
			}
		} break;
		case ':': {
			mode = Mode::COMMAND;
			updateModeline();
			command_line.contents[0] = Character{CharColor::GREEN, CharColor::BLACK, ":"};
			command_line.draw();
		} break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			command_number += input[0];
			break;
		default: {
			return false;
		}
	}
	return true;
}

bool Application::processInsertInput() {
	size_t char_diff = 0;
	switch (input[0]) {
		case 27: {
			if (strcmp(input, "\033[3~") == 0) {
				char_diff = text_buffer->removeFront(1);
			} else {
				return false;
			}
		} break;
		default: {
			if (!iscntrl(input[0])) {
				char_diff = text_buffer->insert(input, 1);
			} else if (input[0] == '\t') {
				char_diff = text_buffer->insert(input, 1);
			} else if (input[0] == 0x0D) {
				size_t scope_count = text_buffer->scopeCount();
				if (scope_count > 0) {
					char ins_string[32] = "\n";
					memset(&ins_string[1], (int)'\t', scope_count);
					char_diff += text_buffer->insert(ins_string, scope_count + 1);
				} else {
					char_diff += text_buffer->insert("\n", 1);
				}
			} else if (input[0] == 127) {
				char_diff = text_buffer->removeBack(1);
			} else {
				return false;
			}
		}
	}
	if (char_diff && !modified) {
		modified = true;
		updateModeline();
	}
	return true;
}

bool Application::processSelectInput() {
	switch (input[0]) {
		case 0x0D: {
			if (command == "y") {
				text_buffer->getSelection();
				text_buffer->cancelSelection();
			} else if (command == "d") {
				text_buffer->getSelection();
				text_buffer->deleteSelection();
				text_buffer->cancelSelection();
				modified = true;
			}
			mode = Mode::NORMAL;
			updateModeline();
			command = "";
		} break;
		case 127: {
			text_buffer->deleteSelection();
			text_buffer->cancelSelection();
			modified = true;
			mode = Mode::NORMAL;
			updateModeline();
			command = "";
		} break;
		case 'j': {
			if (command_number == "") {
				text_buffer->down(1);
			} else {
				text_buffer->down(std::stoul(command_number, nullptr));
				command_number = "";
			}
		} break;
		case 'k': {
			if (command_number == "") {
				text_buffer->up(1);
			} else {
				text_buffer->up(std::stoul(command_number, nullptr));
				command_number = "";
			}
		} break;
		case 'h': {
			if (command_number.empty()) {
				text_buffer->retreat(1);
			} else {
				text_buffer->retreat(std::stoul(command_number, nullptr));
				command_number = "";
			}
		} break;
		case 'l': {
			if (command_number.empty()) {
				text_buffer->advance(1);
			} else {
				text_buffer->advance(std::stoul(command_number, nullptr));
				command_number = "";
			}
		} break;
		case 'm': {
			if (!command_number.empty()) {
				text_buffer->move(std::stoul(command_number, nullptr));
				command_number = "";
			}
		} break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			command_number += input[0];
			break;
		default: {
			return false;
		}
	}
	return true;
}

bool Application::processReplaceInput() {
	switch (input[0]) {
		default: {
			return false;
		}
	}
	return true;
}

bool Application::processCommandInput() {
	switch (input[0]) {
		case 0x0D: processCommand(); break;
		default: {
			if (!iscntrl(input[0])) {
				command += input[0];
				command_line.contents[command.length()] = 
					Character{CharColor::GREEN, CharColor::BLACK, {input[0]}};
			} else if (input[0] == 127) {
				command_line.contents[command.length()] = Character{};
				if (command.length() == 0) {
					mode = Mode::NORMAL;
					updateModeline();
				} else {
					command.pop_back();
				}
			} else {
				return false;
			}
		}
	}
	command_line.draw();
	return true;
}

void Application::processCommand() {
	if (command == "w") {
		FILE *file = fopen(filename.c_str(), "w");
		if (file) {
			text_buffer->saveFile(file);
			fclose(file);
			modified = false;
			debug("wrote to ", filename.c_str());
		}
	} else if (command == "q") {
		running = false;
	} else if (command == "waq") {
		FILE *file = fopen(filename.c_str(), "w");
		text_buffer->saveFile(file);
		fclose(file);
		debug("wrote to ", filename.c_str());
		running = false;
	} else if (command[0] == 'e') {
		std::string file_name = command.substr(2);
		this->filename = file_name;
		Result result = text_buffer->loadBuffer(filename);
		if (result != SUCCESS) {
			Logger::error("failed to open file!");
		}
	}
	command = "";
	for (unsigned int i = 0; i < command_line.width * command_line.height; i++) {
		command_line.contents[i] = Character{};
	}
	mode = Mode::NORMAL;
	updateModeline();
	command_line.draw();
}

bool Application::processGlobalInput() {
	if (strcmp(input, "\033") == 0) {
		mode = Mode::NORMAL;
		updateModeline();
		write(STDOUT_FILENO, "\033[1 q", sizeof("\033[1 q") - 1);
		command = "";
		for (unsigned int i = 0; i < command_line.width * command_line.height; i++) {
			command_line.contents[i] = Character{};
		}
		text_buffer->cancelSelection();
		command_line.draw();
	} else if (strcmp(input, "\033[H") == 0) {
		text_buffer->home();
	} else if (strcmp(input, "\033[F") == 0) {
		text_buffer->end();
	} else if (strcmp(input, "\033[A") == 0) {
		text_buffer->up(1);
	} else if (strcmp(input, "\033[B") == 0) {
		text_buffer->down(1);
	} else if (strcmp(input, "\033[C") == 0) {
		text_buffer->advance(1);
	} else if (strcmp(input, "\033[D") == 0) {
		text_buffer->retreat(1);
	} else if (strncmp(input, "\033]52;c;", sizeof("\033]52;c;") - 1) == 0) {
		char buffer[4096] = {0};
		size_t len = sizeof("\033]52;c;") - 1;
		long length = strlen(input);
		memmove(input, &input[len], 4095 - len);
		length += read(STDIN_FILENO, &input[4095 - len], len - 3);
		input[length] = '\0';
		toBytes(input, buffer);
		text_buffer->insert(buffer, strlen(buffer));
		length = read(STDIN_FILENO, input, 4092);
		while (length > 0) {
			input[length] = '\0';
			toBytes(input, buffer);
			text_buffer->insert(buffer, strlen(buffer));
			length = read(STDIN_FILENO, input, 4092);
		}
		modified = true;
		updateModeline();
	} else {
		debug("invalid input: ", input);
	}
	return true;
}