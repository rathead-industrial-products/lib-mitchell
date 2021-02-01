/*******************************************************************************
 *    INCLUDED FILES
 ******************************************************************************/

//-- unity: unit test framework
#include "unity.h"

//-- module being tested
#include "printf-emb.h"


/*******************************************************************************
 *    DEFINITIONS
 ******************************************************************************/

/*******************************************************************************
 *    PRIVATE TYPES
 ******************************************************************************/

/*******************************************************************************
 *    PRIVATE DATA
 ******************************************************************************/
char buf[128];


/*******************************************************************************
 *    PRIVATE FUNCTIONS
 ******************************************************************************/
static void resetTest(void) {
    tearDown();
    setUp();
}

/*******************************************************************************
 *    SETUP, TEARDOWN
 ******************************************************************************/
#include <string.h>
void setUp(void)
{
    memcpy(buf, "Invalid String", strlen("Invalid String") + 1);
}

void tearDown(void)
{
}

/*******************************************************************************
 *    TESTS
 ******************************************************************************/

void test_truncation_and_return_value(void) {
    TEST_ASSERT_EQUAL_INT(3, snprintf (buf,  0, "abc")); TEST_ASSERT_EQUAL_STRING("Invalid String", buf); resetTest();
    TEST_ASSERT_EQUAL_INT(3, snprintf (NULL, 0, "abc")); TEST_ASSERT_EQUAL_STRING("Invalid String", buf); resetTest();
    TEST_ASSERT_EQUAL_INT(0, snprintf (buf,  5, ""));    TEST_ASSERT_EQUAL_STRING("",               buf); resetTest();
    TEST_ASSERT_EQUAL_INT(3, snprintf (buf,  1, "abc")); TEST_ASSERT_EQUAL_STRING("",    buf); TEST_ASSERT_EQUAL_HEX8('\0', buf[0]); resetTest();
    TEST_ASSERT_EQUAL_INT(3, snprintf (buf,  2, "abc")); TEST_ASSERT_EQUAL_STRING("a",   buf); TEST_ASSERT_EQUAL_HEX8('\0', buf[1]); resetTest();
    TEST_ASSERT_EQUAL_INT(3, snprintf (buf,  3, "abc")); TEST_ASSERT_EQUAL_STRING("ab",  buf); TEST_ASSERT_EQUAL_HEX8('\0', buf[2]); resetTest();
    TEST_ASSERT_EQUAL_INT(3, snprintf (buf,  4, "abc")); TEST_ASSERT_EQUAL_STRING("abc", buf); TEST_ASSERT_EQUAL_HEX8('\0', buf[3]); resetTest();
    TEST_ASSERT_EQUAL_INT(3, snprintf (buf,  5, "abc")); TEST_ASSERT_EQUAL_STRING("abc", buf); TEST_ASSERT_EQUAL_HEX8('\0', buf[3]); resetTest();
}
void test_d_basic_formatting(void) {
    TEST_ASSERT_EQUAL_INT(1, snprintf (buf, 128, "%d",       5)); TEST_ASSERT_EQUAL_STRING("5",     buf); resetTest();
    TEST_ASSERT_EQUAL_INT(1, snprintf (buf, 128, "%d",       0)); TEST_ASSERT_EQUAL_STRING("0",     buf); resetTest();
    TEST_ASSERT_EQUAL_INT(0, snprintf (buf, 128, "%.0d",     0)); TEST_ASSERT_EQUAL_STRING("",      buf); resetTest();
    TEST_ASSERT_EQUAL_INT(0, snprintf (buf, 128, "%5.0d",    0)); TEST_ASSERT_EQUAL_STRING("",      buf); resetTest();
    TEST_ASSERT_EQUAL_INT(1, snprintf (buf, 128, "%.0d",     1)); TEST_ASSERT_EQUAL_STRING("1",     buf); resetTest();
    TEST_ASSERT_EQUAL_INT(1, snprintf (buf, 128, "%.d",      2)); TEST_ASSERT_EQUAL_STRING("2",     buf); resetTest();
    TEST_ASSERT_EQUAL_INT(2, snprintf (buf, 128, "%d",      -1)); TEST_ASSERT_EQUAL_STRING("-1",    buf); resetTest();
    TEST_ASSERT_EQUAL_INT(3, snprintf (buf, 128, "%.3d",     5)); TEST_ASSERT_EQUAL_STRING("005",   buf); resetTest();
    TEST_ASSERT_EQUAL_INT(4, snprintf (buf, 128, "%.3d",    -5)); TEST_ASSERT_EQUAL_STRING("-005",  buf); resetTest();
    TEST_ASSERT_EQUAL_INT(5, snprintf (buf, 128, "%5.3d",    5)); TEST_ASSERT_EQUAL_STRING("  005", buf); resetTest();
    TEST_ASSERT_EQUAL_INT(5, snprintf (buf, 128, "%5.3d",   -5)); TEST_ASSERT_EQUAL_STRING(" -005", buf); resetTest();
    TEST_ASSERT_EQUAL_INT(5, snprintf (buf, 128, "%05.3d",  -5)); TEST_ASSERT_EQUAL_STRING(" -005", buf); resetTest();
    TEST_ASSERT_EQUAL_INT(5, snprintf (buf, 128, "%-5.3d",  -5)); TEST_ASSERT_EQUAL_STRING("-005 ", buf); resetTest();
    TEST_ASSERT_EQUAL_INT(5, snprintf (buf, 128, "%-5.-3d", -5)); TEST_ASSERT_EQUAL_STRING("-5   ", buf); resetTest();
}
void test_d_length_modifiers(void) {
    TEST_ASSERT_EQUAL_INT(2, snprintf (buf, 128, "%hhd", (signed char)        -5)); TEST_ASSERT_EQUAL_STRING("-5", buf); resetTest();
    TEST_ASSERT_EQUAL_INT(1, snprintf (buf, 128, "%hhd", (unsigned char)       5)); TEST_ASSERT_EQUAL_STRING("5",  buf); resetTest();
    TEST_ASSERT_EQUAL_INT(2, snprintf (buf, 128, "%hd",  (short)              -5)); TEST_ASSERT_EQUAL_STRING("-5", buf); resetTest();
    TEST_ASSERT_EQUAL_INT(1, snprintf (buf, 128, "%hd",  (unsigned short)      5)); TEST_ASSERT_EQUAL_STRING("5",  buf); resetTest();
    TEST_ASSERT_EQUAL_INT(2, snprintf (buf, 128, "%ld",  (long)               -5)); TEST_ASSERT_EQUAL_STRING("-5", buf); resetTest();
    TEST_ASSERT_EQUAL_INT(1, snprintf (buf, 128, "%ld",  (unsigned long)       5)); TEST_ASSERT_EQUAL_STRING("5",  buf); resetTest();
    TEST_ASSERT_EQUAL_INT(2, snprintf (buf, 128, "%lld", (long long)          -5)); TEST_ASSERT_EQUAL_STRING("-5", buf); resetTest();
    TEST_ASSERT_EQUAL_INT(1, snprintf (buf, 128, "%lld", (unsigned long long)  5)); TEST_ASSERT_EQUAL_STRING("5",  buf); resetTest();
    TEST_ASSERT_EQUAL_INT(1, snprintf (buf, 128, "%zd",  (size_t)              5)); TEST_ASSERT_EQUAL_STRING("5",  buf); resetTest();
}
void test_d_flags(void) {
    TEST_ASSERT_EQUAL_INT(1, snprintf (buf, 128, "%-d",    5)); TEST_ASSERT_EQUAL_STRING("5",   buf); resetTest();
    TEST_ASSERT_EQUAL_INT(2, snprintf (buf, 128, "%-+d",   5)); TEST_ASSERT_EQUAL_STRING("+5",  buf); resetTest();
    TEST_ASSERT_EQUAL_INT(2, snprintf (buf, 128, "%+-d",   5)); TEST_ASSERT_EQUAL_STRING("+5",  buf); resetTest();
    TEST_ASSERT_EQUAL_INT(2, snprintf (buf, 128, "%+d",   -5)); TEST_ASSERT_EQUAL_STRING("-5",  buf); resetTest();
    TEST_ASSERT_EQUAL_INT(2, snprintf (buf, 128, "% d",    5)); TEST_ASSERT_EQUAL_STRING(" 5",  buf); resetTest();
    TEST_ASSERT_EQUAL_INT(1, snprintf (buf, 128, "% .0d",  0)); TEST_ASSERT_EQUAL_STRING(" ",   buf); resetTest();
    TEST_ASSERT_EQUAL_INT(2, snprintf (buf, 128, "% +d",   5)); TEST_ASSERT_EQUAL_STRING("+5",  buf); resetTest();
    TEST_ASSERT_EQUAL_INT(3, snprintf (buf, 128, "%03d",   5)); TEST_ASSERT_EQUAL_STRING("005", buf); resetTest();
    TEST_ASSERT_EQUAL_INT(3, snprintf (buf, 128, "%-03d", -5)); TEST_ASSERT_EQUAL_STRING("-5 ", buf); resetTest();
    TEST_ASSERT_EQUAL_INT(3, snprintf (buf, 128, "%3d",   -5)); TEST_ASSERT_EQUAL_STRING(" -5", buf); resetTest();
    TEST_ASSERT_EQUAL_INT(3, snprintf (buf, 128, "%03d",  -5)); TEST_ASSERT_EQUAL_STRING("-05", buf); resetTest();
}
void test_o_basic_formatting(void) {
    TEST_ASSERT_EQUAL_INT(1, snprintf (buf, 128, "%o",    5)); TEST_ASSERT_EQUAL_STRING("5",     buf); resetTest();
    TEST_ASSERT_EQUAL_INT(2, snprintf (buf, 128, "%o",    8)); TEST_ASSERT_EQUAL_STRING("10",    buf); resetTest();
    TEST_ASSERT_EQUAL_INT(1, snprintf (buf, 128, "%o",    0)); TEST_ASSERT_EQUAL_STRING("0",     buf); resetTest();
    TEST_ASSERT_EQUAL_INT(2, snprintf (buf, 128, "%#o",   5)); TEST_ASSERT_EQUAL_STRING("05",    buf); resetTest();
    TEST_ASSERT_EQUAL_INT(2, snprintf (buf, 128, "%#o",  05)); TEST_ASSERT_EQUAL_STRING("05",    buf); resetTest();
    TEST_ASSERT_EQUAL_INT(0, snprintf (buf, 128, "%.0o",  0)); TEST_ASSERT_EQUAL_STRING("",      buf); resetTest();
    TEST_ASSERT_EQUAL_INT(1, snprintf (buf, 128, "%.0o",  1)); TEST_ASSERT_EQUAL_STRING("1",     buf); resetTest();
    TEST_ASSERT_EQUAL_INT(3, snprintf (buf, 128, "%.3o",  5)); TEST_ASSERT_EQUAL_STRING("005",   buf); resetTest();
    TEST_ASSERT_EQUAL_INT(3, snprintf (buf, 128, "%.3o",  8)); TEST_ASSERT_EQUAL_STRING("010",   buf); resetTest();
    TEST_ASSERT_EQUAL_INT(5, snprintf (buf, 128, "%5.3o", 5)); TEST_ASSERT_EQUAL_STRING("  005", buf); resetTest();
}
void test_u_basic_formatting(void) {
    TEST_ASSERT_EQUAL_INT(1, snprintf (buf, 128, "%u",    5)); TEST_ASSERT_EQUAL_STRING("5",     buf); resetTest();
    TEST_ASSERT_EQUAL_INT(1, snprintf (buf, 128, "%u",    0)); TEST_ASSERT_EQUAL_STRING("0",     buf); resetTest();
    TEST_ASSERT_EQUAL_INT(0, snprintf (buf, 128, "%.0u",  0)); TEST_ASSERT_EQUAL_STRING("",      buf); resetTest();
    TEST_ASSERT_EQUAL_INT(0, snprintf (buf, 128, "%5.0u", 0)); TEST_ASSERT_EQUAL_STRING("",      buf); resetTest();
    TEST_ASSERT_EQUAL_INT(1, snprintf (buf, 128, "%.0u",  1)); TEST_ASSERT_EQUAL_STRING("1",     buf); resetTest();
    TEST_ASSERT_EQUAL_INT(3, snprintf (buf, 128, "%.3u",  5)); TEST_ASSERT_EQUAL_STRING("005",   buf); resetTest();
    TEST_ASSERT_EQUAL_INT(5, snprintf (buf, 128, "%5.3u", 5)); TEST_ASSERT_EQUAL_STRING("  005", buf); resetTest();
}
void test_x_basic_formatting(void) {
    TEST_ASSERT_EQUAL_INT(1, snprintf (buf, 128, "%x",    5)); TEST_ASSERT_EQUAL_STRING("5",     buf); resetTest();
    TEST_ASSERT_EQUAL_INT(2, snprintf (buf, 128, "%x",   31)); TEST_ASSERT_EQUAL_STRING("1f",    buf); resetTest();
    TEST_ASSERT_EQUAL_INT(1, snprintf (buf, 128, "%x",    0)); TEST_ASSERT_EQUAL_STRING("0",     buf); resetTest();
    TEST_ASSERT_EQUAL_INT(0, snprintf (buf, 128, "%.0x",  0)); TEST_ASSERT_EQUAL_STRING("",      buf); resetTest();
    TEST_ASSERT_EQUAL_INT(0, snprintf (buf, 128, "%5.0x", 0)); TEST_ASSERT_EQUAL_STRING("",      buf); resetTest();
    TEST_ASSERT_EQUAL_INT(1, snprintf (buf, 128, "%.0x",  1)); TEST_ASSERT_EQUAL_STRING("1",     buf); resetTest();
    TEST_ASSERT_EQUAL_INT(3, snprintf (buf, 128, "%.3x",  5)); TEST_ASSERT_EQUAL_STRING("005",   buf); resetTest();
    TEST_ASSERT_EQUAL_INT(3, snprintf (buf, 128, "%.3x", 31)); TEST_ASSERT_EQUAL_STRING("01f",   buf); resetTest();
    TEST_ASSERT_EQUAL_INT(5, snprintf (buf, 128, "%5.3x", 5)); TEST_ASSERT_EQUAL_STRING("  005", buf); resetTest();
}
void test_x_flags(void) {
    TEST_ASSERT_EQUAL_INT(1, snprintf (buf, 128, "%-x",  5)); TEST_ASSERT_EQUAL_STRING("5",    buf); resetTest();
    TEST_ASSERT_EQUAL_INT(3, snprintf (buf, 128, "%-3x", 5)); TEST_ASSERT_EQUAL_STRING("5  ",  buf); resetTest();
    TEST_ASSERT_EQUAL_INT(3, snprintf (buf, 128, "%03x", 5)); TEST_ASSERT_EQUAL_STRING("005",  buf); resetTest();
    TEST_ASSERT_EQUAL_INT(4, snprintf (buf, 128, "%#x", 31)); TEST_ASSERT_EQUAL_STRING("0x1f", buf); resetTest();
    TEST_ASSERT_EQUAL_INT(1, snprintf (buf, 128, "%#x",  0)); TEST_ASSERT_EQUAL_STRING("0",    buf); resetTest();
}
void test_X_basic_formatting(void) {
    TEST_ASSERT_EQUAL_INT(1, snprintf (buf, 128, "%X",    5)); TEST_ASSERT_EQUAL_STRING("5",     buf); resetTest();
    TEST_ASSERT_EQUAL_INT(2, snprintf (buf, 128, "%X",   31)); TEST_ASSERT_EQUAL_STRING("1F",    buf); resetTest();
    TEST_ASSERT_EQUAL_INT(1, snprintf (buf, 128, "%X",    0)); TEST_ASSERT_EQUAL_STRING("0",     buf); resetTest();
    TEST_ASSERT_EQUAL_INT(0, snprintf (buf, 128, "%.0X",  0)); TEST_ASSERT_EQUAL_STRING("",      buf); resetTest();
    TEST_ASSERT_EQUAL_INT(0, snprintf (buf, 128, "%5.0X", 0)); TEST_ASSERT_EQUAL_STRING("",      buf); resetTest();
    TEST_ASSERT_EQUAL_INT(1, snprintf (buf, 128, "%.0X",  1)); TEST_ASSERT_EQUAL_STRING("1",     buf); resetTest();
    TEST_ASSERT_EQUAL_INT(3, snprintf (buf, 128, "%.3X",  5)); TEST_ASSERT_EQUAL_STRING("005",   buf); resetTest();
    TEST_ASSERT_EQUAL_INT(3, snprintf (buf, 128, "%.3X", 31)); TEST_ASSERT_EQUAL_STRING("01F",   buf); resetTest();
    TEST_ASSERT_EQUAL_INT(5, snprintf (buf, 128, "%5.3X", 5)); TEST_ASSERT_EQUAL_STRING("  005", buf); resetTest();
}
void test_X_flags(void) {
    TEST_ASSERT_EQUAL_INT(1, snprintf (buf, 128, "%-X",  5)); TEST_ASSERT_EQUAL_STRING("5",    buf); resetTest();
    TEST_ASSERT_EQUAL_INT(3, snprintf (buf, 128, "%03X", 5)); TEST_ASSERT_EQUAL_STRING("005",  buf); resetTest();
    TEST_ASSERT_EQUAL_INT(4, snprintf (buf, 128, "%#X", 31)); TEST_ASSERT_EQUAL_STRING("0X1F", buf); resetTest();
    TEST_ASSERT_EQUAL_INT(1, snprintf (buf, 128, "%#X",  0)); TEST_ASSERT_EQUAL_STRING("0",    buf); resetTest();
}
void test_p_basic_formatting(void) {
    TEST_ASSERT_EQUAL_INT(5,  snprintf (buf, 128, "%p",       1234)); TEST_ASSERT_EQUAL_STRING("0x4d2",      buf); resetTest();
    TEST_ASSERT_EQUAL_INT(10, snprintf (buf, 128, "%.8p",     1234)); TEST_ASSERT_EQUAL_STRING("0x000004d2", buf); resetTest();
    TEST_ASSERT_EQUAL_INT(10, snprintf (buf, 128, "%p", 0x20001ffc)); TEST_ASSERT_EQUAL_STRING("0x20001ffc", buf); resetTest();
    TEST_ASSERT_EQUAL_INT(10, snprintf (buf, 128, "%.8p",        0)); TEST_ASSERT_EQUAL_STRING("0x00000000", buf); resetTest();
    TEST_ASSERT_EQUAL_INT(10, snprintf (buf, 128, "%.6p",       -1)); TEST_ASSERT_EQUAL_STRING("0xffffffff", buf); resetTest();
}
void test_e_basic_formatting(void) {
    TEST_ASSERT_EQUAL_INT(12, snprintf (buf, 128, "%e",   314159265)); TEST_ASSERT_EQUAL_STRING("3.141593e+08",   buf); resetTest();
    TEST_ASSERT_EQUAL_INT(14, snprintf (buf, 128, "%.8e", 314159265)); TEST_ASSERT_EQUAL_STRING("3.14159265e+08", buf); resetTest();
    TEST_ASSERT_EQUAL_INT(5,  snprintf (buf, 128, "%.0e", 314159265)); TEST_ASSERT_EQUAL_STRING("3e+08",          buf); resetTest();
    TEST_ASSERT_EQUAL_INT(1,  snprintf (buf, 128, "%.0e",         0)); TEST_ASSERT_EQUAL_STRING("0",              buf); resetTest();
    TEST_ASSERT_EQUAL_INT(7,  snprintf (buf, 128, "%.1e",         0)); TEST_ASSERT_EQUAL_STRING("0.0e+00",        buf); resetTest();
}
void test_e_flags(void) {
    TEST_ASSERT_EQUAL_INT(13, snprintf (buf, 128, "%+e",    314159265)); TEST_ASSERT_EQUAL_STRING("+3.141593e+08", buf); resetTest();
    TEST_ASSERT_EQUAL_INT(13, snprintf (buf, 128, "% e",    314159265)); TEST_ASSERT_EQUAL_STRING(" 3.141593e+08", buf); resetTest();
    TEST_ASSERT_EQUAL_INT(6,  snprintf (buf, 128, "%#.0e",  314159265)); TEST_ASSERT_EQUAL_STRING("3.e+08",        buf); resetTest();
    TEST_ASSERT_EQUAL_INT(13, snprintf (buf, 128, "%e",    -314159265)); TEST_ASSERT_EQUAL_STRING("-3.141593e+08", buf); resetTest();
    TEST_ASSERT_EQUAL_INT(9,  snprintf (buf, 128, "%09.2e", 314159265)); TEST_ASSERT_EQUAL_STRING("03.14e+08",     buf); resetTest();
    TEST_ASSERT_EQUAL_INT(8,  snprintf (buf, 128, "%.2e",        9999)); TEST_ASSERT_EQUAL_STRING("1.00e+04",      buf); resetTest();
    TEST_ASSERT_EQUAL_INT(8,  snprintf (buf, 128, "%.2e",          12)); TEST_ASSERT_EQUAL_STRING("1.20e+01",      buf); resetTest();
    TEST_ASSERT_EQUAL_INT(8,  snprintf (buf, 128, "%.3E",        1234)); TEST_ASSERT_EQUAL_STRING("1.23e+03",      buf); resetTest();
    TEST_ASSERT_EQUAL_INT(9,  snprintf (buf, 128, "%.4E",       12345)); TEST_ASSERT_EQUAL_STRING("12.35e+03",     buf); resetTest();
    TEST_ASSERT_EQUAL_INT(10, snprintf (buf, 128, "%.5E",      123456)); TEST_ASSERT_EQUAL_STRING("123.46e+03",    buf); resetTest();
    TEST_ASSERT_EQUAL_INT(11, snprintf (buf, 128, "%E",        123456)); TEST_ASSERT_EQUAL_STRING("123.456e+03",   buf); resetTest();
    TEST_ASSERT_EQUAL_INT(7,  snprintf (buf, 128, "%.0E",      123456)); TEST_ASSERT_EQUAL_STRING("100e+03",       buf); resetTest();
    TEST_ASSERT_EQUAL_INT(7,  snprintf (buf, 128, "%.1E",      123456)); TEST_ASSERT_EQUAL_STRING("100e+03",       buf); resetTest();
    TEST_ASSERT_EQUAL_INT(8,  snprintf (buf, 128, "%#.1E",     123456)); TEST_ASSERT_EQUAL_STRING("100.e+03",      buf); resetTest();
    TEST_ASSERT_EQUAL_INT(7,  snprintf (buf, 128, "%.2E",      123456)); TEST_ASSERT_EQUAL_STRING("120e+03",       buf); resetTest();
    TEST_ASSERT_EQUAL_INT(7,  snprintf (buf, 128, "%.3E",      123567)); TEST_ASSERT_EQUAL_STRING("124e+03",       buf); resetTest();
    TEST_ASSERT_EQUAL_INT(7,  snprintf (buf, 128, "%.2E",     1234567)); TEST_ASSERT_EQUAL_STRING("1.2e+06",       buf); resetTest();
    TEST_ASSERT_EQUAL_INT(7,  snprintf (buf, 128, "%.2E",       99999)); TEST_ASSERT_EQUAL_STRING("100e+03",       buf); resetTest();
    TEST_ASSERT_EQUAL_INT(7,  snprintf (buf, 128, "%.3E",       99999)); TEST_ASSERT_EQUAL_STRING("100e+03",       buf); resetTest();
    TEST_ASSERT_EQUAL_INT(7,  snprintf (buf, 128, "%.2E",      999999)); TEST_ASSERT_EQUAL_STRING("1.0e+06",       buf); resetTest();
}
void test_c(void) {
    TEST_ASSERT_EQUAL_INT(1, snprintf (buf, 128, "%c", 'a')); TEST_ASSERT_EQUAL_STRING("a", buf); resetTest();
}
void test_s(void) {
    TEST_ASSERT_EQUAL_INT(2,  snprintf (buf, 128, "%.2s",         "abc")); TEST_ASSERT_EQUAL_STRING("ab",    buf); resetTest();
    TEST_ASSERT_EQUAL_INT(3,  snprintf (buf, 128, "%.6s",         "abc")); TEST_ASSERT_EQUAL_STRING("abc",   buf); resetTest();
    TEST_ASSERT_EQUAL_INT(5,  snprintf (buf, 128, "%5s",          "abc")); TEST_ASSERT_EQUAL_STRING("  abc", buf); resetTest();
    TEST_ASSERT_EQUAL_INT(5,  snprintf (buf, 128, "%-5s",         "abc")); TEST_ASSERT_EQUAL_STRING("abc  ", buf); resetTest();
    TEST_ASSERT_EQUAL_INT(5,  snprintf (buf, 128, "%5.2s",        "abc")); TEST_ASSERT_EQUAL_STRING("   ab", buf); resetTest();
    TEST_ASSERT_EQUAL_INT(5,  snprintf (buf, 128, "%*s",   5,     "abc")); TEST_ASSERT_EQUAL_STRING("  abc", buf); resetTest();
    TEST_ASSERT_EQUAL_INT(5,  snprintf (buf, 128, "%*s",  -5,     "abc")); TEST_ASSERT_EQUAL_STRING("abc  ", buf); resetTest();
    TEST_ASSERT_EQUAL_INT(5,  snprintf (buf, 128, "%*.*s", 5, 2,  "abc")); TEST_ASSERT_EQUAL_STRING("   ab", buf);                         // do not reset buf
    TEST_ASSERT_EQUAL_INT(16, snprintf (buf, 128, "%s + cat_text",  buf)); TEST_ASSERT_EQUAL_STRING("   ab + cat_text", buf); resetTest(); // this is allowed in contravention to the spec
    TEST_ASSERT_EQUAL_INT(5,  snprintf (buf, 128, "%*.*s", 5, 2,  "abc")); TEST_ASSERT_EQUAL_STRING("   ab", buf);                         // do not reset buf
    TEST_ASSERT_EQUAL_INT(16, snprintf (buf, 128, "cat_text + %s",  buf)); TEST_ASSERT_EQUAL_STRING("cat_text +    ab", buf); resetTest();
}
void test_pct_pct(void) {
    TEST_ASSERT_EQUAL_INT(1, snprintf (buf, 128, "%%")); TEST_ASSERT_EQUAL_STRING("%", buf); resetTest();
    TEST_ASSERT_EQUAL_INT(1, snprintf (buf, 128, "%?")); TEST_ASSERT_EQUAL_STRING("%", buf); resetTest();
}
void test_64_bit_support(void) {
    TEST_ASSERT_EQUAL_INT(6,  snprintf (buf, 128, "%lli",  (int64_t)        123456)); TEST_ASSERT_EQUAL_STRING("123456",               buf); resetTest();
    TEST_ASSERT_EQUAL_INT(7,  snprintf (buf, 128, "%lli",  (int64_t)       -123456)); TEST_ASSERT_EQUAL_STRING("-123456",              buf); resetTest();
    TEST_ASSERT_EQUAL_INT(6,  snprintf (buf, 128, "%llu",  (uint64_t)       123456)); TEST_ASSERT_EQUAL_STRING("123456",               buf); resetTest();
    TEST_ASSERT_EQUAL_INT(6,  snprintf (buf, 128, "%llo",  (int64_t)        123456)); TEST_ASSERT_EQUAL_STRING("361100",               buf); resetTest();
    TEST_ASSERT_EQUAL_INT(7,  snprintf (buf, 128, "%#llo", (int64_t)        123456)); TEST_ASSERT_EQUAL_STRING("0361100",              buf); resetTest();
    TEST_ASSERT_EQUAL_INT(5,  snprintf (buf, 128, "%llx",  (int64_t)        123456)); TEST_ASSERT_EQUAL_STRING("1e240",                buf); resetTest();
    TEST_ASSERT_EQUAL_INT(7,  snprintf (buf, 128, "%#llx", (int64_t)        123456)); TEST_ASSERT_EQUAL_STRING("0x1e240",              buf); resetTest();
    TEST_ASSERT_EQUAL_INT(5,  snprintf (buf, 128, "%llX",  (int64_t)        123456)); TEST_ASSERT_EQUAL_STRING("1E240",                buf); resetTest();
    TEST_ASSERT_EQUAL_INT(16, snprintf (buf, 128, "%llx",                     -1ll)); TEST_ASSERT_EQUAL_STRING("ffffffffffffffff",     buf); resetTest();
    TEST_ASSERT_EQUAL_INT(2,  snprintf (buf, 128, "%lld",                     -1ll)); TEST_ASSERT_EQUAL_STRING("-1",                   buf); resetTest();
    TEST_ASSERT_EQUAL_INT(20, snprintf (buf, 128, "%llu",                     -1ll)); TEST_ASSERT_EQUAL_STRING("18446744073709551615", buf); resetTest();
    TEST_ASSERT_EQUAL_INT(13, snprintf (buf, 128, "%lle",                     -1ll)); TEST_ASSERT_EQUAL_STRING("-1.000000e+00",        buf); resetTest();
    TEST_ASSERT_EQUAL_INT(8,  snprintf (buf, 128, "%.3llE",  1234567890123456789ll)); TEST_ASSERT_EQUAL_STRING("1.23e+18",             buf); resetTest();
    TEST_ASSERT_EQUAL_INT(9,  snprintf (buf, 128, "%.3llE", -1234567890123456789ll)); TEST_ASSERT_EQUAL_STRING("-1.23e+18",            buf); resetTest();
    TEST_ASSERT_EQUAL_INT(7,  snprintf (buf, 128, "%.3llE",          99953577184ll)); TEST_ASSERT_EQUAL_STRING("100e+09",              buf); resetTest();
    TEST_ASSERT_EQUAL_INT(8,  snprintf (buf, 128, "%.3llE",         999535771840ll)); TEST_ASSERT_EQUAL_STRING("1.00e+12",             buf); resetTest();
}

