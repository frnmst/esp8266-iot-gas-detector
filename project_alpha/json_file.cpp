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
#include "json_file.h"

/* Loads a json file directly into a document object. */
bool readJson(fs::FS &fs, const char * path, JsonDocument &doc)
{
    File file = fs.open(path, "r");
    if (!file || file.isDirectory())
    {
        return false;
    }

    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error)
    {
        prt("deserialization error: ");
        prtn(error.c_str());
        return false;
    }

    return true;
}

bool writeJson(fs::FS &fs, const char * path, const JsonDocument &doc)
{
    File file = fs.open(path, "w");
    if (!file)
    {
        prt("error opening file for writing: ");
        prtn(path);
        return false;
    }

    if (serializeJson(doc, file) == 0)
    {
        prtn("failed to write json to file");
        file.close();
        return false;
    }

    file.close();
    return true;
}