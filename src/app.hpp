#pragma once

#include "defines.hpp"

#include "text_buffer.hpp"

#include <termios.h>
#include <string>

enum class Mode {
	NORMAL,
	INSERT,
	SELECT,
	REPLACE,
	COMMAND,
};

class Application {
public:
	~Application();

	Result init(const char *filename);
	void run();

private:
	void updateModeline();
	void processInput();
	bool processNormalInput();
	bool processInsertInput();
	bool processSelectInput();
	bool processReplaceInput();
	bool processCommandInput();
	void processCommand();
	bool processGlobalInput();

	bool running = false;
	Mode mode = Mode::NORMAL;
	Frame modeline;
	Frame command_line;

	termios terminal_settings;
	std::string command;
	std::string filename;
	std::string command_number;
	TextBuffer *text_buffer = nullptr;
	char input[4096] = {0};
	bool modified = false;
};