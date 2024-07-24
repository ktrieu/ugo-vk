#pragma once

#include <optional>

#include <fmt/format.h>
#include <vulkan/vulkan.h>

class Logger {
public:
	static void initialize();
	static Logger& get();

	template <typename... T>
	void log(fmt::format_string<T...> fmt_string, T&&... args);
private:
	static std::optional<Logger> instance;
};

template <typename... T>
void Logger::log(fmt::format_string<T...> fmt_string, T&&... args)
{
	fmt::println(fmt_string, std::forward<T>(args)...);
}

template <typename... T>
static void log(fmt::format_string<T...> fmt_string, T&&... args)
{
	Logger::get().log(fmt_string, std::forward<T>(args)...);
}

void log_glfw_error();