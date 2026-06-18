/* version 0.15.0
 *
 * MIT License
 *
 * Copyright (c) 2024-2026 Franco Masotti
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "utils.h"

// 1. Valid inputs.
test(getCurrentDateTime_standard_case)
{
    struct tm time_info;
    time_info.tm_year = 126; // Years since 1900 (2026)
    time_info.tm_mon = 4;    // May (0-11)
    time_info.tm_mday = 24;  // Day
    time_info.tm_hour = 20;
    time_info.tm_min = 0;
    time_info.tm_sec = 0;

    char buffer[32];
    assertTrue(getCurrentDateTime(&time_info, buffer, sizeof(buffer), "%Y-%m-%d"));
    assertEqual(buffer, "2026-05-24");
}

// 2. Buffer cannot hold the resulting string + null terminator.
test(getCurrentDateTime_buffer_too_small)
{
    struct tm time_info = {0};
    time_info.tm_year = 126; // 2026
    
    // "2026" requires 4 chars + 1 null terminator = 5 bytes.
    // Providing a length of 4 will force strftime to fail and return 0.
    char buffer[4] = {0}; 
    assertFalse(getCurrentDateTime(&time_info, buffer, 4, "%Y"));
    assertEqual(buffer, "202");
}

// 3. Verify the exact memory boundary.
test(getCurrentDateTime_exact_buffer_size)
{
    struct tm time_info = {0};
    time_info.tm_year = 126; 

    // "2026" needs exactly 5 bytes (4 chars + '\0')
    char buffer[5];
    assertTrue(getCurrentDateTime(&time_info, buffer, 5, "%Y"));
    assertEqual(buffer, "2026");
}

// 4. Verify behavior when format is empty.
test(getCurrentDateTime_empty_format)
{
    struct tm time_info = {0};
    char buffer[10] = "X"; 

    // According to C standard, empty format returns 0 and clears the buffer
    assertFalse(getCurrentDateTime(&time_info, buffer, sizeof(buffer), ""));
    assertEqual(buffer, "");
}

// 5. Verify behavior when length is explicitly 0.
test(getCurrentDateTime_length_zero)
{
    struct tm time_info = {0};
    char buffer[10] = "original";

    // Original buffer contents must remain untouched.
    assertFalse(getCurrentDateTime(&time_info, buffer, 0, "%Y"));
    assertEqual(buffer, "original");
}

// 6. struct tm is NULL.
test(getCurrentDateTime_null_time_pointer)
{
    char buffer[16] = "start";
    
    // The buffer should be safely handled (e.g., cleared) instead of crashing.
    assertFalse(getCurrentDateTime(NULL, buffer, sizeof(buffer), "%Y"));
    assertEqual(buffer, "");
}

// 7. Verify when destination buffer is NULL.
test(getCurrentDateTime_null_buffer_pointer)
{
    struct tm time_info = {0};
    
    assertFalse(getCurrentDateTime(&time_info, NULL, 16, "%Y"));
}

// 9. Verify when format string is NULL.
test(getCurrentDateTime_null_format_pointer)
{
    struct tm time_info = {0};
    char buffer[16] = "Initial";

    assertFalse(getCurrentDateTime(&time_info, buffer, sizeof(buffer), NULL));
    assertEqual(buffer, "");
}