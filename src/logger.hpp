#pragma once

#include "defines.hpp"

#include <cstdio>
#include <source_location>
#include <utility>
#include <typeinfo>

class Logger {
public:
	static Result init(const char *log_file_name = "/home/andrew/Code/Yadda/v3/yadda.log");
	static void deinit();

	static void fatal(const char *message = "");
	static void error(const char *message = "");
	static void warn(const char *message = "");
	static void info(const char *message = "");
	template <typename T> static void logParam(T t) {
		fprintf(log_file, "unknown param of type: %s, located at: %p", typeid(T).name(), (void *)t);
	}
	template <typename... Args> static void _debug(const std::source_location location, Args... args) {
		// doesn't need #ifndef preamble since this function doesn't get called outside of debug mode
		if (log_file == nullptr) {
			printf("can I see the error!\n");
			return; // TODO: handle this error!
		}
		fprintf(log_file, "[DEBUG]:[%s]:[%s]:[%i,%i]:", location.file_name(), location.function_name(), location.line(), location.column());
	}

private:
	static FILE *log_file;
};

#ifndef NDEBUG
#define debug(...) Logger::_debug(std::source_location::current(), __VA_ARGS__)
#define assert(condition, result, message) if (!condition) { Logger::error(message); return result; }
#else
#define debug(...)
#define assert(condition, result, message)
#endif
