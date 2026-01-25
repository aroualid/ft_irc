#include "mocks/MockOutput.hpp"
#include <criterion/criterion.h>
#include <criterion/hooks.h>
#include <criterion/logging.h>

// Hook that runs before each test - clear the mock output buffer and temp file
ReportHook(PRE_TEST)(struct criterion_test* test)
{
	(void)test;
	MockOutput::getInstance().clear();
}

// Hook that runs after each test - dump buffer if test failed
// Reads from temp file since tests run in child processes
ReportHook(POST_TEST)(struct criterion_test_stats* stats)
{
	bool testFailed = (stats->failed_asserts > 0 || stats->timed_out || stats->signal != 0);
	if (testFailed && MockOutput::fileHasOutput())
	{
		std::string buffer = MockOutput::readFromFile();
		cr_log_error("--- Mock Output ---\n%s--- End Mock Output ---", buffer.c_str());
	}
	// Clear the temp file for next test
	MockOutput::getInstance().clear();
}
