#pragma once
#include <memory.h>

typedef char str;
#define ALLOCATOR(size) malloc(size)

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
StringLength(str *string)
{
    return ((string_header *)(string) - 1)->length;
}

inline static size_t 
StringCapacity(str *string)
{
    return ((string_header *)(string) - 1)->capacity;
}

inline static string_header *
StringGetHeader(str *string)
{
    return (string_header *)(string) - 1;
}

static str *
StringAllocate(size_t size)
{
    string_header *newstring_header = 
        (string_header *)ALLOCATOR(sizeof(string_header) + sizeof(str) * size + 1);
    str *newString = (str *)(newstring_header + 1);

    newstring_header->length = size;
    newstring_header->capacity = size + 1;
    newString[size] = '\0';

    return newString;
}

static str *
StringCreate(char *text)
{
    if(text == 0) return 0;

    size_t charLength = CharLength(text);
    str *newString = StringAllocate(charLength);

    memcpy(newString, text, charLength);

    return newString;
}

static void
StringFree(str *string)
{
    string_header *stringHeader = ((string_header *)string) - 1;
    free(stringHeader);
}

static str *
StringCreateSubstring(char *text, size_t length)
{
    size_t textSize = 0;
    while(textSize != length && text[textSize] != '\0')
    {
        textSize++;
    }
    str *newString = StringAllocate(textSize);

    memcpy(newString, text, textSize);

    return newString;
}

static str *
StringConcatChar(str *string, char *text)
{
    size_t stringLength = StringLength(string);
    size_t charLength = CharLength(text);
    str *newString = StringAllocate(stringLength + charLength);

    memcpy(newString, string, stringLength);
    memcpy(newString + stringLength, text, charLength);

    return newString;
}

static str *
StringsConcat(str *string1, str *string2)
{
    size_t string1Length = StringLength(string1);
    size_t string2Length = StringLength(string2);

    str *newString = StringAllocate(string1Length + string2Length);

    memcpy(newString, string1, string1Length);
    memcpy(newString + string1Length, string2, string2Length);

    return newString;
}

//
//  Formatting
//

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
StringToLowerCase(str *string)
{
    for(size_t i = 0; i != StringLength(string); i++)
        if(CharIsUpperCase(string[i])) string[i] += 32;
}

static void
StringToCamelCase(str *string)
{
    if(string[0] && CharIsUpperCase(string[0])) string[0] += 32;
}

//
//  Conditionals
//

static bool32
StringsMatch(str *a, str *b)
{
    if(!(a && b)) return false;
    size_t aLength = StringLength(a);
    size_t bLength = StringLength(b);
    if(aLength != bLength) return false;
    
    return memcmp(a, b, aLength) == 0;
}

// TEST: 
static bool32
StringsMatchIgnoreCase(str *a, str *b)
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
StringMatchChar(str *a, char *b)
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
StringContainsSubstring(str *string, str *substring)
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
StringMatchesAnyInCharList(str *string, char *list[], int numberOfItems)
{
    for(int i = 0; i != numberOfItems; i++)
    {
        if(StringMatchChar(string, list[i])) return true;
    }
    return false;
}
