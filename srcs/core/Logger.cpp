#include "core/Logger.hpp"
#include <iostream>

Logger::Logger()
	: m_minLevel(INFO), m_output(&std::cout), m_currentLevel(INFO), m_lineStarted(false)
{
}

Logger::~Logger()
{
	flush();
}

Logger& Logger::getInstance()
{
	static Logger instance;
	return (instance);
}

void Logger::setMinLevel(Level level)
{
	m_minLevel = level;
}

void Logger::setOutput(std::ostream& output)
{
	m_output = &output;
}

Logger& Logger::log(Level level)
{
	flush();
	m_currentLevel = level;
	m_lineStarted = false;
	return (*this);
}

Logger& Logger::debug()
{
	return log(DEBUG);
}
Logger& Logger::notice()
{
	return log(NOTICE);
}
Logger& Logger::info()
{
	return log(INFO);
}
Logger& Logger::warning()
{
	return log(WARNING);
}
Logger& Logger::error()
{
	return log(ERROR);
}
Logger& Logger::critical()
{
	return log(CRITICAL);
}

std::string Logger::getLevelString(Level level) const
{
	switch (level)
	{
	case DEBUG:
		return "DEBUG";
	case NOTICE:
		return "NOTICE";
	case INFO:
		return "INFO";
	case WARNING:
		return "WARN";
	case ERROR:
		return "ERROR";
	case CRITICAL:
		return "CRIT";
	default:
		return "?????"; // if you got a better suggestion im all ears
	}
}

std::string Logger::getColorCode(Level level) const
{
	switch (level)
	{
	case DEBUG:
		return "\033[36m"; // cyan
	case NOTICE:
		return "\033[96m"; // bright cyan
	case INFO:
		return "\033[32m"; // green
	case WARNING:
		return "\033[33m"; // yellow
	case ERROR:
		return "\033[31m"; // red
	case CRITICAL:
		return "\033[95m"; // bold Red
	default:
		return "\033[0m"; // reset
	}
}

void Logger::flush()
{
	if (m_lineStarted && m_currentLevel >= m_minLevel)
	{
		std::ostream& out = (m_currentLevel >= ERROR) ? std::cerr : *m_output;
		out << m_buffer.str() << "\033[0m" << '\n';
		out.flush();
	}
	m_buffer.str("");
	m_buffer.clear();
	m_lineStarted = false;
}

Logger& Logger::operator<<(std::ostream& (*manip)(std::ostream&))
{
	if (manip == static_cast< std::ostream& (*)(std::ostream&) >(std::endl))
	{
		flush();
	}
	return *this;
}
