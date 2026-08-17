#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

static bool is_palindrome(const char *text)
{
    if (text == NULL)
    {
        return false;
    }

    size_t length = strlen(text);


    if (length < 2U)
    {
        return true;
    }

    size_t left = 0U;
    size_t right = length - 1U;

    while (left < right)
    {
        if (text[left] != text[right])
        {
            return false;
        }

        ++left;
        --right;
    }

    return true;
}

int main(void)
{
    assert(is_palindrome(""));
    assert(is_palindrome("a"));
    assert(is_palindrome("level"));
    assert(is_palindrome("abba"));

    assert(!is_palindrome("OPEN"));
    assert(!is_palindrome("hello"));
    assert(!is_palindrome(NULL));

    puts("palindrome tests passed");
    return 0;
}