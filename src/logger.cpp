#include "logger.hpp"

FILE *Logger::log_file = nullptr;

Result Logger::init(const char *log_file_name) {
#ifndef NDEBUG
	if (log_file_name == nullptr) {
		return NULL_ERROR;
	} else if (log_file != nullptr) {
		return IO_ERROR;
	}
#endif
	log_file = fopen(log_file_name, "w+");
	if (log_file == nullptr) {
		return IO_ERROR;
	}
	info("logger initialized.");
	return SUCCESS;
}

void Logger::deinit() {
#ifndef NDEBUG
	if (log_file == nullptr) {
		return;
	}
#endif
	info("logger shutdown");
	fflush(log_file);
	fclose(log_file);
	log_file = nullptr;
}

void Logger::fatal(const char *message) {
#ifndef NDEBUG
	if (log_file == nullptr) {
		return; // TODO: report error
	} else if (message == nullptr) {
		error("invalid logging message");
		return;
	}
#endif
	fprintf(log_file, "[FATAL]:%s\n", message);
}

void Logger::error(const char *message) {
#ifndef NDEBUG
	if (log_file == nullptr) {
		return; // TODO: report error
	} else if (message == nullptr) {
		error("invalid logging message");
		return;
	}
#endif
	fprintf(log_file, "[ERROR]:%s\n", message);
}

void Logger::warn(const char *message) {
#ifndef NDEBUG
	if (log_file == nullptr) {
		return; // report error
	} else if (message == nullptr) {
		error("invalid logging message");
		return;
	}
#endif
	fprintf(log_file, "[WARN]:%s\n", message);
}

void Logger::info(const char *message) {
#ifndef NDEBUG
	if (log_file == nullptr) {
		return; // TODO: report error
	} else if (message == nullptr) {
		error("invalid logging message");
		return;
	}
#endif
	fprintf(log_file, "[INFO]:%s\n", message);
}