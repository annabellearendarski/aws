#include <assert.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "awsString.h"
#include "errorCodes.h"

/*!
 * Count the number of leading zeros in an unsigned integer.
 *
 * \param[in]	value							Integer value, must not be zero.
 * \return										Number of leading zeros.
 */
static size_t
awsStringCountLeadingZeros(size_t value)
{
    size_t result;

    assert(value != 0);

#if defined(__GNUC__) || defined(__clang__)
    result = __builtin_clzl((unsigned long)value);
#else
    size_t count;
    size_t tmp;

    tmp = value;
    count = 0;

    while (tmp != 0) {
        tmp >>= 1;
        count++;
    }

    result = (sizeof(size_t) * 8) - count;
#endif // defined(__GNUC__) || defined(__clang__)

    return result;
}

/*
 * Compute the capacity required to store a string of the given size, in bytes.
 *
 * The size includes the terminating null character.
 *
 * The size must not be zero.
 *
 */
static errorCode
awsStringComputeCapacity(const struct awsString *pString, size_t size,
                         size_t *pCapacity)
{
    errorCode error;

    assert(pString);

    //
    // The computed capacity is the power-of-two equal to or immediately greater
    // than the size. In other words, it's the value with a single bit set where
    // the index of that bit is one more than the MSB bit of the size, unless
    // the size is already a power-of-two.
    //
    // If the size already requires all bits of the size_t type to be encoded,
    // then there's no extra bit available to encode the capacity.
    //

    if (size <= ((size_t)1 << ((sizeof(size_t) * 8) - 1))) {
        *pCapacity = (size_t)1 << ((sizeof(size_t) * 8) -
                                   awsStringCountLeadingZeros(size));

        error = NO_ERROR;
    } else {
        error = INVALID_PARAMETER;
    }

    return error;
}

/*!
 * Resize the buffer of a string.
 *
 * This function only resizes the buffer, it doesn't guarantee the content is
 * null-terminated.
 *
 * If an allocation error occurs, the string is left unchanged.
 *
 * If an allocation error occurs, and the new size is smaller than the current
 * size, the operation is considered successful.
 *
 * \param[in]	pString						String.
 * \param[in]	size						Required size, in bytes.
 * \return									aws_NO_ERROR if successful.
 */
static errorCode
awsStringResizeBuffer(struct awsString *pString, size_t size)
{
    errorCode error;

    assert(pString);

    size_t capacity;

    //
    // XXX If a user performs frequent operations on a string that change its
    // size below and above the size of the internal buffer, the performance
    // impact of copying the content to and from the internal buffer would cause
    // hysteresis.
    //
    // There is no special handling of this situation as it is assumed that the
    // internal buffer is small enough that the cost of the copy operation is
    // negligible.
    //

    error = awsStringComputeCapacity(pString, size, &capacity);

    if (error == NO_ERROR) {

        if (capacity != pString->capacity) {

            if (capacity <= sizeof(pString->internalBuffer)) {

                if (pString->pBuffer != pString->internalBuffer) {
                    memcpy(pString->internalBuffer, pString->pBuffer, capacity);
                    free(pString->pBuffer);

                    pString->pBuffer = pString->internalBuffer;
                    pString->capacity = sizeof(pString->internalBuffer);
                }
            } else {

                if (pString->pBuffer == pString->internalBuffer) {
                    char *pBuffer;

                    pBuffer = malloc(capacity);

                    if (pBuffer) {
                        memcpy(pBuffer, pString->internalBuffer,
                               pString->capacity);
                        pString->pBuffer = pBuffer;
                        pString->capacity = capacity;
                    } else {
                        error = MALLOC_FAILED;
                    }
                } else {
                    char *pBuffer;

                    pBuffer = realloc(pString->pBuffer, capacity);

                    if (pBuffer) {
                        pString->pBuffer = pBuffer;
                        pString->capacity = capacity;
                    } else {
                        if (capacity < pString->capacity) {
                            error = NO_ERROR;
                        } else {
                            error = MALLOC_FAILED;
                        }
                    }
                }
            }
        }
    }
    return error;
}

/*!
 * Assign a buffer of characters to a string.
 *
 * \param[in]	pString						String.
 * \param[in]	pBuffer						Buffer.
 * \param[in]	length						Length of the buffer, in bytes.
 * \return									NO_ERROR if successful.
 */
errorCode
awsStringAssignBuffer(struct awsString *pString, const char *pBuffer, size_t length)
{
    assert(pString);
    assert(pBuffer);

    if (pString->errorCode == NO_ERROR) {
        pString->errorCode = awsStringResizeBuffer(pString, length + 1);

        if (pString->errorCode == NO_ERROR) {
            memcpy(pString->pBuffer, pBuffer, length);
            pString->pBuffer[length] = '\0';
            pString->length = length;
        }
    }

    return pString->errorCode;
}

errorCode
awsStringAppend(struct awsString *pString, const struct awsString *pAppendString)
{
    assert(pString);
    assert(pAppendString);

    if (pString->errorCode == NO_ERROR) {
        size_t length;

        length = pString->length + pAppendString->length;

        pString->errorCode = awsStringResizeBuffer(pString, length + 1);

        if (pString->errorCode == NO_ERROR) {
            memcpy(&pString->pBuffer[pString->length], pAppendString->pBuffer,
                   pAppendString->length + 1);
            pString->length = length;
        }
    }

    return pString->errorCode;
}

void
awsStringConstructEmpty(struct awsString *pString)
{
    assert(pString);

    pString->internalBuffer[0] = '\0';

    pString->pBuffer = pString->internalBuffer;
    pString->capacity = sizeof(pString->internalBuffer);
    pString->length = 0;

    pString->errorCode = NO_ERROR;
}

void
awsStringDestroy(struct awsString *pString)
{
    assert(pString);

    if (pString->pBuffer != pString->internalBuffer) {
        free(pString->pBuffer);
    }
}

errorCode
awsStringAssignVF(struct awsString *pString, const char *pFormat, va_list args)
{
    assert(pString);
    assert(pFormat);

    if (pString->errorCode == NO_ERROR) {
        va_list argsCopy;
        int result;

        va_copy(argsCopy, args);
        result =
            vsnprintf(pString->pBuffer, pString->capacity, pFormat, argsCopy);
        assert(result >= 0);
        va_end(argsCopy);

        if ((size_t)result < pString->capacity) {
            pString->length = (size_t)result;
        } else {
            pString->errorCode =
                awsStringResizeBuffer(pString, (size_t)result + 1);

            if (pString->errorCode == NO_ERROR) {
                vsnprintf(pString->pBuffer, pString->capacity, pFormat, args);
                pString->length = (size_t)result;
            }
        }
    }

    return pString->errorCode;
}

errorCode
awsStringConstructF(struct awsString *pString, const char *pFormat, ...)
{
    errorCode error;
    va_list args;

    awsStringConstructEmpty(pString);

    va_start(args, pFormat);
    error = awsStringAssignVF(pString, pFormat, args);
    va_end(args);

    return error;
}

errorCode
awsStringAppendVF(struct awsString *pString, const char *pFormat, va_list args)
{
	assert(pString);
	assert(pFormat);

	if (pString->errorCode == NO_ERROR)
	{
		va_list							 argsCopy;
		int								 result;
		size_t							 remainingSize;
		size_t							 newLength;

		remainingSize = pString->capacity - pString->length;

		va_copy(argsCopy, args);
		result = vsnprintf(&pString->pBuffer[pString->length], remainingSize, pFormat, argsCopy);
		assert(result >= 0);
		va_end(argsCopy);

		newLength = pString->length + (size_t)result;

		if ((size_t)result < remainingSize)
		{
			pString->length = newLength;
		}
		else
		{
			pString->errorCode = awsStringResizeBuffer(pString, newLength + 1);

			if (pString->errorCode == NO_ERROR)
			{
				remainingSize = pString->capacity - pString->length;
				vsnprintf(&pString->pBuffer[pString->length], remainingSize, pFormat, args);
				pString->length = newLength;
			}
		}
	}

	return pString->errorCode;
}

errorCode
awsStringAppendF(struct awsString *pString, const char *pFormat, ...)
{
    assert(pString);
    assert(pFormat);

    if (pString->errorCode == NO_ERROR) {
        va_list args;

        va_start(args, pFormat);
        awsStringAppendVF(pString, pFormat, args);
        va_end(args);
    }

    return pString->errorCode;
}

errorCode
awsStringAppendBuffer(struct awsString *pString, const char *pBuffer,
                      size_t buffer_length)
{
    assert(pString);
    assert(pBuffer);

    if (pString->errorCode == NO_ERROR) {
        pString->errorCode = awsStringResizeBuffer(pString, buffer_length + 1);

        if (pString->errorCode == NO_ERROR) {
            memcpy(pString->pBuffer, pBuffer, buffer_length);
            pString->pBuffer[buffer_length] = '\0';
            pString->length = buffer_length;
        }
    }

    return pString->errorCode;
}
