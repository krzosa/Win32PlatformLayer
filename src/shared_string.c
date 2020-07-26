#pragma once
#include <memory.h>
#include <malloc.h>


typedef char str8;
#define STR_ALLOC(size) malloc(size)


/* TODO: 
    Optional string pool implmentation
    Capacity bigger then length
 */

typedef struct string_header
{
    size_t length;
    size_t capacity;
} string_header;

inline static size_t
CharLength(char *text)
{
    size_t stringSize = 0;
    for(size_t i = 0; text[i] != '\0'; i++)
        stringSize++;
    return stringSize;
}

inline static size_t 
StringLength(str8 *string)
{
    return ((string_header *)(string) - 1)->length;
}

inline static size_t 
StringCapacity(str8 *string)
{
    return ((string_header *)(string) - 1)->capacity;
}

inline static string_header *
StringGetHeader(str8 *string)
{
    return (string_header *)(string) - 1;
}

static str8 *
StringAllocate(size_t size)
{
    string_header *newstring_header = 
        (string_header *)STR_ALLOC(sizeof(string_header) + sizeof(str8) * size + 1);
    str8 *newString = (str8 *)(newstring_header + 1);

    newstring_header->length = size;
    newstring_header->capacity = size + 1;
    newString[size] = '\0';

    return newString;
}

static str8 *
StringCreate(char *text)
{
    if(text == 0) return 0;

    size_t charLength = CharLength(text);
    str8 *newString = StringAllocate(charLength);

    memcpy(newString, text, charLength);

    return newString;
}

static void
StringFree(str8 *string)
{
    string_header *stringHeader = ((string_header *)string) - 1;
    free(stringHeader);
}

static str8 *
StringCreateSubstring(char *text, size_t length)
{
    size_t textSize = 0;
    while(textSize != length && text[textSize] != '\0')
    {
        textSize++;
    }
    str8 *newString = StringAllocate(textSize);

    memcpy(newString, text, textSize);

    return newString;
}

static str8 *
StringConcatChar(str8 *string, char *text)
{
    size_t stringLength = StringLength(string);
    size_t charLength = CharLength(text);
    str8 *newString = StringAllocate(stringLength + charLength);

    memcpy(newString, string, stringLength);
    memcpy(newString + stringLength, text, charLength);

    return newString;
}

static str8 *
StringsConcat(str8 *string1, str8 *string2)
{
    size_t string1Length = StringLength(string1);
    size_t string2Length = StringLength(string2);

    str8 *newString = StringAllocate(string1Length + string2Length);

    memcpy(newString, string1, string1Length);
    memcpy(newString + string1Length, string2, string2Length);

    return newString;
}

//
//  Formatting
//

#if 0
#include <stdio.h>
#include <stdarg.h>
// Formatting of text with variables to 'embed'
// WARNING: String returned will expire after this function is called MAX_TEXTFORMAT_BUFFERS times
char *TextFormat(char *text, ...)
{
    #define MAX_TEXT_FORMAT_BUFFER 2
    #define MAX_TEXT_FORMAT_BUFFER_SIZE 1024
    // We create an array of buffers so strings don't expire until MAX_TEXTFORMAT_BUFFERS invocations
    static char buffers[MAX_TEXT_FORMAT_BUFFER][MAX_TEXT_FORMAT_BUFFER_SIZE] = { 0 };
    static int  index = 0;

    char *currentBuffer = buffers[index];
    memset(currentBuffer, 0, MAX_TEXT_FORMAT_BUFFER_SIZE);   // Clear buffer before using

    va_list args;
    va_start(args, text);
    vsprintf(currentBuffer, text, args);
    va_end(args);

    index += 1;     // Move to next buffer for next function call
    if (index >= MAX_TEXT_FORMAT_BUFFER) index = 0;

    return currentBuffer;
}
#endif

static bool32
CharIsUpperCase(char character)
{
    if(character >= 65 && character <= 90) return true;
    return false;
}

static bool32
CharIsLowerCase(char character)
{
    if(character >= 97 && character <= 122) return true;
    return false;
}

static void
StringToLowerCase(str8 *string)
{
    for(size_t i = 0; i != StringLength(string); i++)
        if(CharIsUpperCase(string[i])) string[i] += 32;
}

static void
StringToCamelCase(str8 *string)
{
    if(string[0] && CharIsUpperCase(string[0])) string[0] += 32;
}

//
//  Conditionals
//

static bool32
StringsMatch(str8 *a, str8 *b)
{
    if(!(a && b)) return false;
    size_t aLength = StringLength(a);
    size_t bLength = StringLength(b);
    if(aLength != bLength) return false;
    
    return memcmp(a, b, aLength) == 0;
}

// TEST: 
static bool32
StringsMatchIgnoreCase(str8 *a, str8 *b)
{
    if(!(a&&b) || StringLength(a) != StringLength(b)) return false;

    char tempA, tempB;
    for(size_t i = 0; i != StringLength(a); i++)
    {
        tempA = a[i]; tempB = b[i];
        if(CharIsUpperCase(a[i])) tempA += 32;
        if(CharIsUpperCase(b[i])) tempB += 32;
        if(tempA != tempB) return false;
    }
    return true;
}

static bool32
StringMatchChar(str8 *a, char *b)
{
    size_t aLength = StringLength(a);
    size_t bLength = CharLength(b);
    if(aLength != bLength) return false;

    return memcmp(a, b, aLength) == 0;
}

static bool32
CharsMatch(char *a, char *b)
{
    // return false if a or b is null
    if(!(a && b)) return false;

    for(size_t i = 0; a[i] && b[i]; i++)
    {
        if(a[i] != b[i]) return false;
    }
    return true;
}

static bool32
CharsMatchIgnoreCase(char *a, char *b)
{
    // return false if a or b is null
    if(!(a && b)) return false;

    char tempA, tempB;
    for(size_t i = 0; a[i] && b[i]; i++)
    {
        tempA = a[i]; tempB = b[i];
        if(CharIsUpperCase(a[i])) tempA += 32;
        if(CharIsUpperCase(b[i])) tempB += 32;
        if(tempA != tempB) return false;
    }
    return true;
}

static bool32
StringContainsSubstring(str8 *string, str8 *substring)
{
    if(StringLength(substring) > StringLength(string)) return false;

    for(size_t i = 0; i < StringLength(string);i++)
    {
        if(string[i] == substring[0])
        {
            for(size_t j = 0;;)
            {
                if(substring[j] == '\0') return true;
                else if(substring[j] == string[i+j]) j++;
                else break;
            }
        }
    }
    return false;
}

// NOTE: I guess you cant really figure out the size of 
// a static array using sizeof at runtime
static bool32
StringMatchesAnyInCharList(str8 *string, char *list[], int numberOfItems)
{
    for(int i = 0; i != numberOfItems; i++)
    {
        if(StringMatchChar(string, list[i])) return true;
    }
    return false;
}
