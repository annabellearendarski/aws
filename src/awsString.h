/*!
 * \file		sbgString.h
 * \ingroup		common
 * \author		SBG Systems
 * \date		March 20, 2020
 *
 * \brief		Character string.
 *
 * \copyright		Copyright (C) 2022, SBG Systems SAS. All rights reserved.
 * \beginlicense	The MIT license
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
 *
 * \endlicense
 */

#ifndef AWSSTRING_H
#define AWSSTRING_H

#include <stdarg.h>
#include <stddef.h>

#include "errorCodes.h"

#define AWS_CONFIG_STRING_INTERNAL_BUFFER_SIZE 32

struct awsString {
    char internalBuffer[AWS_CONFIG_STRING_INTERNAL_BUFFER_SIZE];
    char *pBuffer;
    size_t capacity;
    size_t length;
    errorCode errorCode;
};

void awsStringConstructEmpty(struct awsString *pString);

void awsStringDestroy(struct awsString *pString);

errorCode awsStringAssignVF(struct awsString *pString, const char *pFormat, va_list args);

errorCode awsStringConstructF(struct awsString *pString, const char *pFormat, ...);

errorCode awsStringAppend(struct awsString *pString, const struct awsString *pAppendString);

errorCode awsStringAppendF(struct awsString *pString, const char *pFormat, ...);

errorCode awsStringAppendVF(struct awsString *pString, const char *pFormat, va_list args);

errorCode awsStringAssignBuffer(struct awsString *pString, const char *pBuffer, size_t length);

errorCode awsStringAppendBuffer(struct awsString *pString, const char *pBuffer, size_t buffer_length);

#endif /* AWSSTRING_H */
