#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool is_open_bracket(char ch)
{
    return ch == '(' || ch == '[' || ch == '{';
}

static bool is_close_bracket(char ch)
{
    return ch == ')' || ch == ']' || ch == '}';
}

static bool is_matching(char left, char right)
{
    return (left == '(' && right == ')') ||
           (left == '[' && right == ']') ||
           (left == '{' && right == '}');
}

static bool is_valid_parentheses(const char *text)
{
    if (text == NULL)
    {
        return false;
    }

    size_t length = strlen(text);
    size_t capacity = (length == 0U) ? 1U : length;

    char *stack = malloc(capacity);

    if (stack == NULL)
    {
        return false;
    }

    size_t top = 0U;
    bool valid = true;

    for (size_t index = 0U; index < length; ++index)
    {
        char current = text[index];

        if (is_open_bracket(current))
        {
          
            stack[top++] = current;
        }
        else if (is_close_bracket(current))
        {
           
            if (top == 0U)
            {
                valid = false;
                break;
            }

        
            char left = stack[--top];

            if (!is_matching(left, current))
            {
                valid = false;
                break;
            }
        }
        else
        {
       
            valid = false;
            break;
        }
    }


    if (top != 0U)
    {
        valid = false;
    }

    free(stack);
    return valid;
}

int main(void)
{
    assert(is_valid_parentheses(""));
    assert(is_valid_parentheses("()"));
    assert(is_valid_parentheses("()[]{}"));
    assert(is_valid_parentheses("{[()]}"));

    assert(!is_valid_parentheses("(]"));
    assert(!is_valid_parentheses("([)]"));
    assert(!is_valid_parentheses("("));
    assert(!is_valid_parentheses("]"));
    assert(!is_valid_parentheses("abc"));
    assert(!is_valid_parentheses(NULL));

    printf("valid parentheses tests passed\n");

    return 0;
}