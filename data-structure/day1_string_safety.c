#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

static bool safe_string_copy(char *destination,
                             size_t capacity,
                             const char *source)
{
    if (destination == NULL ||
        source == NULL ||
        capacity == 0U)
    {
        return false;
    }

    size_t index = 0U;

    
    while (source[index] != '\0' &&
           index + 1U < capacity)
    {
        destination[index] = source[index];
        ++index;
    }

    destination[index] = '\0';

   
    return source[index] == '\0';
}

int main(void)
{
    char command[] = "OPEN";
    char buffer[8];

    printf("sizeof(command) = %zu\n", sizeof(command));
    printf("strlen(command) = %zu\n", strlen(command));

    bool complete = safe_string_copy(
        buffer,
        sizeof(buffer),
        command
    );

    assert(complete);
    assert(strcmp(buffer, "OPEN") == 0);

    printf("normal copy = %s\n", buffer);

    complete = safe_string_copy(
        buffer,
        sizeof(buffer),
        "COMMAND_TOO_LONG"
    );

    assert(!complete);
    assert(buffer[sizeof(buffer) - 1U] == '\0');

    printf("truncated copy = %s\n", buffer);

    puts("string safety tests passed");
    return 0;
}