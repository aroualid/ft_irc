#pragma once

#include <fstream>
#include <sstream>
#include <string>

// Singleton that buffers mock output and writes to file for cross-process access
class MockOutput
{
public:
	static MockOutput& getInstance()
	{
		static MockOutput instance;
		return instance;
	}

	// Log a message to the buffer and file
	void log(const std::string& msg)
	{
		m_buffer << msg << "\n";
		// Also write to temp file for cross-process access
		std::ofstream file(getTempFilePath().c_str(), std::ios::app);
		if (file.is_open())
		{
			file << msg << "\n";
			file.close();
		}
	}

	// Clear the buffer and temp file (call in test setup)
	void clear()
	{
		m_buffer.str("");
		m_buffer.clear();
		// Clear the temp file
		std::ofstream file(getTempFilePath().c_str(), std::ios::trunc);
		file.close();
	}

	// Get the buffered output (for displaying on failure)
	std::string getBuffer() const
	{
		return m_buffer.str();
	}

	// Read from temp file (for hook to use)
	static std::string readFromFile()
	{
		std::ifstream file(getTempFilePath().c_str());
		if (!file.is_open())
			return "";
		std::stringstream buffer;
		buffer << file.rdbuf();
		return buffer.str();
	}

	// Check if there's any buffered output
	bool hasOutput() const
	{
		return !m_buffer.str().empty();
	}

	// Check if temp file has content
	static bool fileHasOutput()
	{
		std::ifstream file(getTempFilePath().c_str());
		return file.is_open() && file.peek() != std::ifstream::traits_type::eof();
	}

	static std::string getTempFilePath()
	{
		return "/tmp/mock_output.txt";
	}

private:
	MockOutput()
	{
	}
	MockOutput(const MockOutput&);
	MockOutput& operator=(const MockOutput&);

	std::ostringstream m_buffer;
};

// Convenience macro for logging
#define MOCK_LOG(msg)                                                                              \
	do                                                                                             \
	{                                                                                              \
		std::ostringstream oss;                                                                    \
		oss << "[MOCK] " << msg;                                                                   \
		MockOutput::getInstance().log(oss.str());                                                  \
	} while (0)
