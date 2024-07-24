#include "logger.h"

#include <stdexcept>

#include <GLFW/glfw3.h>

std::optional<Logger> Logger::instance = std::nullopt;

void Logger::initialize()
{
	Logger::instance = Logger();
}

Logger& Logger::get()
{
	if (!Logger::instance.has_value())
	{
		throw std::runtime_error("Logger not initialized.");
	}

	return Logger::instance.value();
}

void log_glfw_error()
{
	const char* err_string;
	glfwGetError(&err_string);
	log("GLFW error: {}", err_string);
}