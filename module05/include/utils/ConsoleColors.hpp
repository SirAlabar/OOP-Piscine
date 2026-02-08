#ifndef CONSOLECOLORS_HPP
#define CONSOLECOLORS_HPP

#include <string>

// ANSI color codes for terminal output
namespace Color
{
	// Reset
	const std::string RESET = "\033[0m";
	
	// Regular colors
	const std::string BLACK = "\033[30m";
	const std::string RED = "\033[31m";
	const std::string GREEN = "\033[32m";
	const std::string YELLOW = "\033[33m";
	const std::string BLUE = "\033[34m";
	const std::string MAGENTA = "\033[35m";
	const std::string CYAN = "\033[36m";
	const std::string WHITE = "\033[37m";
	
	// Bold colors
	const std::string BOLD_RED = "\033[1;31m";
	const std::string BOLD_GREEN = "\033[1;32m";
	const std::string BOLD_YELLOW = "\033[1;33m";
	const std::string BOLD_BLUE = "\033[1;34m";
	const std::string BOLD_MAGENTA = "\033[1;35m";
	const std::string BOLD_CYAN = "\033[1;36m";
	const std::string BOLD_WHITE = "\033[1;37m";
	
	// Background colors
	const std::string BG_RED = "\033[41m";
	const std::string BG_GREEN = "\033[42m";
	const std::string BG_YELLOW = "\033[43m";
	const std::string BG_BLUE = "\033[44m";
	const std::string BG_MAGENTA = "\033[45m";
	const std::string BG_CYAN = "\033[46m";
	const std::string BG_WHITE = "\033[47m";
	
	// Text formatting
	const std::string BOLD = "\033[1m";
	const std::string DIM = "\033[2m";
	const std::string UNDERLINE = "\033[4m";
	const std::string BLINK = "\033[5m";
	const std::string REVERSE = "\033[7m";
	
	// Train state colors
	inline std::string getStateColor(const std::string& state)
	{
		if (state == "Idle") return DIM + WHITE;
		if (state == "Accelerating") return GREEN;
		if (state == "Cruising") return BOLD_GREEN;
		if (state == "Braking") return YELLOW;
		if (state == "Stopped") return RED;
		if (state == "Waiting") return MAGENTA;
		if (state == "Emergency") return BOLD_RED + BG_YELLOW;
		return RESET;
	}
	
	// Event type colors
	inline std::string getEventColor(const std::string& eventType)
	{
		if (eventType.find("StationDelay") != std::string::npos) return YELLOW;
		if (eventType.find("TrackMaintenance") != std::string::npos) return CYAN;
		if (eventType.find("SignalFailure") != std::string::npos) return RED;
		if (eventType.find("Weather") != std::string::npos) return BLUE;
		return WHITE;
	}
}

// Progress bar generator
namespace ProgressBar
{
	inline std::string generate(double progress, int width = 30, const std::string& fill = "█", const std::string& empty = "░")
	{
		if (progress < 0.0) progress = 0.0;
		if (progress > 1.0) progress = 1.0;
		
		int filled = static_cast<int>(progress * width);
		std::string bar = "[";
		
		for (int i = 0; i < width; ++i)
		{
			if (i < filled)
				bar += fill;
			else
				bar += empty;
		}
		
		bar += "]";
		return bar;
	}
	
	inline std::string generateColored(double progress, int width = 30)
	{
		std::string bar = "[";
		int filled = static_cast<int>(progress * width);
		
		for (int i = 0; i < width; ++i)
		{
			if (i < filled)
			{
				if (progress < 0.33)
					bar += Color::RED + "█";
				else if (progress < 0.66)
					bar += Color::YELLOW + "█";
				else
					bar += Color::GREEN + "█";
			}
			else
			{
				bar += Color::DIM + "░";
			}
		}
		
		bar += Color::RESET + "]";
		return bar;
	}
}

// Box drawing characters
namespace Box
{
	const std::string TOP_LEFT = "╔";
	const std::string TOP_RIGHT = "╗";
	const std::string BOTTOM_LEFT = "╚";
	const std::string BOTTOM_RIGHT = "╝";
	const std::string HORIZONTAL = "═";
	const std::string VERTICAL = "║";
	const std::string T_DOWN = "╦";
	const std::string T_UP = "╩";
	const std::string T_RIGHT = "╠";
	const std::string T_LEFT = "╣";
	const std::string CROSS = "╬";
	
	inline std::string drawHeader(const std::string& title, int width = 80)
	{
		std::string result = TOP_LEFT;
		int titleLen = title.length();
		int padding = (width - titleLen - 2) / 2;
		
		for (int i = 0; i < padding; ++i) result += HORIZONTAL;
		result += " " + title + " ";
		for (int i = 0; i < width - titleLen - padding - 2; ++i) result += HORIZONTAL;
		result += TOP_RIGHT + "\n";
		
		return result;
	}
	
	inline std::string drawFooter(int width = 80)
	{
		std::string result = BOTTOM_LEFT;
		for (int i = 0; i < width - 2; ++i) result += HORIZONTAL;
		result += BOTTOM_RIGHT + "\n";
		return result;
	}
}

#endif