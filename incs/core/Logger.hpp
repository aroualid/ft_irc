#pragma once

#include <ostream>
#include <sstream>

class Logger
{
public:
	enum Level
	{
		DEBUG = 0,
		NOTICE = 1,
		INFO = 2,
		WARNING = 3,
		ERROR = 4,
		CRITICAL = 5
	};

private:
	Level m_minLevel;
	std::ostream* m_output;
	std::ostringstream m_buffer;
	Level m_currentLevel;
	bool m_lineStarted;

	Logger();
	~Logger();
	Logger(const Logger&);
	Logger& operator=(const Logger&);

	std::string getLevelString(Level level) const;
	std::string getColorCode(Level level) const;
	void flush();

public:
	static Logger& getInstance();

	void setMinLevel(Level level);
	void setOutput(std::ostream& output);

	Logger& log(Level level);
	Logger& debug();
	Logger& notice();
	Logger& info();
	Logger& warning();
	Logger& error();
	Logger& critical();

	template < typename T > Logger& operator<<(const T& value)
	{
		if (!m_lineStarted)
		{
			m_buffer << "[ " << getColorCode(m_currentLevel) << getLevelString(m_currentLevel)
					 << getColorCode((Level)-1) << " ] ";
			m_lineStarted = true;
		}
		m_buffer << value;
		return *this;
	}

	Logger& operator<<(std::ostream& (*manip)(std::ostream&));
};

#define LOG_DEBUG Logger::getInstance().debug()
#define LOG_NOTICE Logger::getInstance().notice()
#define LOG_INFO Logger::getInstance().info()
#define LOG_WARNING Logger::getInstance().warning()
#define LOG_ERROR Logger::getInstance().error()
#define LOG_CRITICAL Logger::getInstance().critical()
