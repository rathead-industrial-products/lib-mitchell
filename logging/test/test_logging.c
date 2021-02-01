/*******************************************************************************
 *    INCLUDED FILES
 ******************************************************************************/

//-- unity: unit test framework
#include "unity.h"

//-- module being tested
#include "logging.h"
#include "SEGGER_RTT.h"


/*******************************************************************************
 *    DEFINITIONS
 ******************************************************************************/
#define MAX_LOG_ENTRY_LEN   64      // must be the same as in logging.c

#define LOGFILE_SIZE  64
#define LOGFILE_END (__LOGFILE_START + __LOGFILE_SIZE)

// mock linker symbols
size_t __LOGFILE_SIZE = LOGFILE_SIZE;
char   __LOGFILE_START[LOGFILE_SIZE];

/*******************************************************************************
 *    PRIVATE TYPES
 ******************************************************************************/

/*******************************************************************************
 *    PRIVATE DATA
 ******************************************************************************/
static char   g_log_str_copy[MAX_LOG_ENTRY_LEN];
static char * g_p_log_str = __LOGFILE_START;


/*******************************************************************************
 *    PRIVATE FUNCTIONS
 ******************************************************************************/

#include <string.h>
/*
 * Copy the log entry from the log to a string.
 */
void copy_log_str(void) {
    char *s = g_log_str_copy;
    do {
        *s = *g_p_log_str++;
        if (g_p_log_str >= LOGFILE_END) { g_p_log_str = __LOGFILE_START; }
    } while (*s++);

}

#define TEST_LOG_ASSERT_EQUAL_STRING(s)                                 \
    do {                                                                \
        copy_log_str();                                                 \
        TEST_ASSERT_EQUAL_STRING(s, g_log_str_copy);                    \
} while (0)                                                             \



/*******************************************************************************
 *    SETUP, TEARDOWN
 ******************************************************************************/
void setUp(void)
{
}

void tearDown(void)
{
}

/*******************************************************************************
 *    TESTS
 ******************************************************************************/

void test_basic_printf_and_wrap(void) {
    log_printf("%s", "12345678901234567890"); TEST_LOG_ASSERT_EQUAL_STRING("12345678901234567890");
    log_printf("%s", "12345678901234567890"); TEST_LOG_ASSERT_EQUAL_STRING("12345678901234567890");
    log_printf("%s", "12345678901234567890"); TEST_LOG_ASSERT_EQUAL_STRING("12345678901234567890");
    log_printf("%s", "12345678901234567890"); TEST_LOG_ASSERT_EQUAL_STRING("12345678901234567890");
}

#if(0)
void test_basic_printf_fl(void) {
    log_printf_fl(__FILE__, __LINE__, "%s", "1234567890");  TEST_LOG_ASSERT_EQUAL_STRING("../lib_mitchell/logging/test/test_logging.c:83 1234567890");
    log_printf_fl(__func__, 5, "%s", "1234567890");         TEST_LOG_ASSERT_EQUAL_STRING("test_basic_printf_fl:5 1234567890");
}

void test_macro(void) {
    LOG_PRINTF_FUNC_LINE("%s", "1234567890"); TEST_LOG_ASSERT_EQUAL_STRING("test_macro:88 1234567890");
}

#endif
