// SPDX-License-Identifier: MIT

#include "sdl2imgshow.h"

typedef struct _var_opt
{
    struct _var_opt *next;
    char *name;
    char *value;
} var_opt;


var_opt *globalVars;


void *ez_malloc(size_t size)
{
    void *data = malloc(size);

    if (data == NULL)
    {
        fprintf(stderr, "Unable to allocate memory. :(\n");
        exit(255);
    }

    memset(data, '\0', size);
    return data;
}


char *ez_strcatn(char *str1, const char *str2, size_t str2_len)
{
    size_t str1_len;

    if (str1 == NULL)
    {
        str1_len = 0;
        str1 = (char*)ez_malloc(str2_len + 1);
    }
    else
    {
        str1_len = strlen(str1);
        str1 = (char*)realloc(str1, str1_len + str2_len + 1);

        if (str1 == NULL)
        {
            fprintf(stderr, "Unable to allocate memory. :(\n");
            exit(255);
        }
    }

    memcpy(str1 + str1_len, str2, str2_len);

    str1[str1_len + str2_len] = 0;

    return str1;
}


// Simple INI reader function
int ini_read(const char *filename, ini_callback callback, void *state)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        fprintf(stderr, "Error opening file: %s\n", filename);
        return 1;
    }

    char line[1024];
    while (fgets(line, sizeof(line), file))
    {
        // Trim leading and trailing whitespace
        char *start = line;
        while (isspace(*start))
            start++;

        char *end = start + strlen(start) - 1;
        while (end > start && isspace(*end))
            end--;
        *(end + 1) = '\0';

        // Ignore empty lines and comments
        if (*start == '\0' || *start == '#')
            continue;

        // Parse key-value pairs
        char *delimiter = strchr(start, '=');

        if (delimiter != NULL)
        {
            *delimiter = '\0';
            char *key = start;
            char *value = delimiter + 1;

            // Trim trailing whitespace from key
            end = key + strlen(key) - 1;
            while (end > key && isspace(*end)) end--;
            *(end + 1) = '\0';

            // Trim leading whitespace from value
            while (isspace(*value)) value++;

            // Handle quoted text
            if (*value == '"')
            {
                char *endQuote = strchr(value + 1, '"');
                if (endQuote != NULL)
                {
                    *endQuote = '\0';
                    value++;
                }
            }

            // Call the callback function
            callback(state, key, value);
        }
    }

    fclose(file);
    return 0;
}


bool file_exists(const char *filename)
{
    FILE *file = fopen(filename, "r");

    if (file != NULL)
    {
        fclose(file);
        return true;
    }

    return false;
}


void init_vars()
{
    globalVars=NULL;
}


void quit_vars()
{
    var_opt *current = globalVars;
    var_opt *next;

    while (current != NULL)
    {
        next = current->next;

        free(current->name);
        free(current->value);
        free(current);

        current = next;
    }

    globalVars = NULL;
}


void set_var(const char *name, const char *value)
{
    var_opt *current = globalVars;

    fprintf(stderr, "%s = %s\n", name, value);

    while (current != NULL)
    {
        if (strcasecmp(name, current->name) == 0)
        {
            free(current->value);
            current->value = strdup(value);
            return;
        }

        current = current->next;
    }

    current = (var_opt*)ez_malloc(sizeof(var_opt));
    current->next = globalVars;
    globalVars = current;

    current->name  = strdup(name);
    current->value = strdup(value);
}


const char *get_var(const char *name)
{
    var_opt *current = globalVars;

    while (current != NULL)
    {
        if (strcasecmp(current->name, name) == 0)
            return current->value;

        current = current->next;
    }

    return getenv(name);
}


char *sub_vars(const char *input)
{   // substitute vars enclosed in {{}}.
    char *output = NULL;

    const char *last = input;
    const char *start = strstr(last, "{{");
    const char *end = NULL;

    fprintf(stderr, "> \"%s\"\n", input);

    while (start != NULL)
    {
        if (start > last)
        {   // copy any text we have just skipped over.
            output = ez_strcatn(output, last, start - last);
        }

        end = strstr(start, "}}");

        if (end == NULL)
        {   // no closing terminator, copy everything from `start` forwards.
            size_t rest = strlen(start);
            output = ez_strcatn(output, start, rest);
            last = start + rest;
            break;
        }

        start += 2;

        // extract the variable name, and the variable value if it exists.
        char *var_name = ez_strcatn(NULL, start, end - start);
        const char* var_value = get_var(var_name);

        fprintf(stderr, "= \"%s\" = ", var_name);

        if (var_value != NULL)
        {   // concatenate the variable value
            fprintf(stderr, "\"%s\"\n", var_value);
            output = ez_strcatn(output, var_value, strlen(var_value));
        }
        else
        {   // concatenate the variable name because what else am i supposed to do?
            fprintf(stderr, "(null)\n");
            output = ez_strcatn(output, var_name, strlen(var_name));
        }

        free(var_name);

        end += 2;

        last = end;
        start = strstr(end, "{{");
    }

    if (last[0] != 0)
    {   // copy anything else left.
        output = ez_strcatn(output, last, strlen(last));
    }

    fprintf(stderr, "< \"%s\"\n", output);

    return output;
}


int get_positon(const char *position)
{
    if (strcasecmp(position, "topleft") == 0)
        return POS_TOPLEFT;

    if (strcasecmp(position, "topcenter") == 0)
        return POS_TOPCENTER;

    if (strcasecmp(position, "topright") == 0)
        return POS_TOPRIGHT;

    if (strcasecmp(position, "midleft") == 0)
        return POS_MIDLEFT;

    if (strcasecmp(position, "center") == 0)
        return POS_CENTER;

    if (strcasecmp(position, "midright") == 0)
        return POS_MIDRIGHT;

    if (strcasecmp(position, "bottomleft") == 0)
        return POS_BOTTOMLEFT;

    if (strcasecmp(position, "bottomcenter") == 0)
        return POS_BOTTOMCENTER;

    if (strcasecmp(position, "bottomright") == 0)
        return POS_BOTTOMRIGHT;

    return POS_CENTER;
}


int get_image_size(const char *size)
{
    if (strcasecmp(size, "fit") == 0)
        return SIZE_FIT;

    if (strcasecmp(size, "vertical") == 0)
        return SIZE_VERTICAL;

    if (strcasecmp(size, "horizontal") == 0)
        return SIZE_HORIZONTAL;

    if (strcasecmp(size, "original") == 0)
        return SIZE_ORIGINAL;

    return SIZE_VERTICAL;
}


int get_text_align(const char *text_align)
{
    if (strcasecmp(text_align, "left") == 0)
        return ALIGN_LEFT;

    if (strcasecmp(text_align, "center") == 0)
        return ALIGN_CENTER;

    if (strcasecmp(text_align, "right") == 0)
        return ALIGN_RIGHT;

    return ALIGN_LEFT;
}


void calculate_texture_size(SDL_Texture *imageTexture, SDL_Rect *textureRect, int size)
{
    // Get the original width and height of the texture
    int originalWidth, originalHeight;
    SDL_QueryTexture(imageTexture, NULL, NULL, &originalWidth, &originalHeight);

    // Set initial width and height
    int textureWidth = originalWidth;
    int textureHeight = originalHeight;

    // Calculate new width and height based on size enum
    switch (size)
    {
    case SIZE_FIT:
        // Fit the texture within screen dimensions while considering margins
        if (originalWidth > screenWidth - globalMargins.x - globalMargins.w)
        {
            textureWidth = screenWidth - globalMargins.x - globalMargins.w;
            textureHeight = (originalHeight * textureWidth) / originalWidth;
        }
        if (textureHeight > screenHeight - globalMargins.y - globalMargins.h)
        {
            textureHeight = screenHeight - globalMargins.y - globalMargins.h;
            textureWidth = (originalWidth * textureHeight) / originalHeight;
        }
        break;

    case SIZE_VERTICAL:
        // Keep original width, adjust height to fit vertically within screen dimensions
        textureHeight = screenHeight - globalMargins.y - globalMargins.h;
        textureWidth = (originalWidth * textureHeight) / originalHeight;
        break;

    case SIZE_HORIZONTAL:
        // Keep original height, adjust width to fit horizontally within screen dimensions
        textureWidth = screenWidth - globalMargins.x - globalMargins.w;
        textureHeight = (originalHeight * textureWidth) / originalWidth;
        break;

    case SIZE_ORIGINAL:
        // Keep original size
        break;

    case SIZE_STRETCH:
        // Stretch the texture to fit screen dimensions
        textureWidth = screenWidth - globalMargins.x - globalMargins.w;
        textureHeight = screenHeight - globalMargins.y - globalMargins.h;
        break;

    default:
        // Default to original size if imageSize not specified
        break;
    }

    // Set the calculated texture width and height to the textureRect
    textureRect->w = textureWidth;
    textureRect->h = textureHeight;
}


void calculate_texture_rect(SDL_Texture *imageTexture, SDL_Rect *textureRect, int position)
{
    UNUSED(imageTexture);

    // Calculate X position based on the position enum
    switch (position)
    {
    case POS_TOPLEFT:
    case POS_MIDLEFT:
    case POS_BOTTOMLEFT:
        textureRect->x = globalMargins.x;
        break;

    case POS_TOPCENTER:
    case POS_CENTER:
    case POS_BOTTOMCENTER:
        textureRect->x = (screenWidth - textureRect->w) / 2;
        break;

    case POS_TOPRIGHT:
    case POS_MIDRIGHT:
    case POS_BOTTOMRIGHT:
        textureRect->x = screenWidth - textureRect->w - globalMargins.w;
        break;

    default:
        textureRect->x = globalMargins.x; // Default to left if position not specified
        break;
    }

    // Calculate Y position based on the position enum
    switch (position)
    {
    case POS_TOPLEFT:
    case POS_TOPCENTER:
    case POS_TOPRIGHT:
        textureRect->y = globalMargins.y;
        break;

    case POS_MIDLEFT:
    case POS_CENTER:
    case POS_MIDRIGHT:
        textureRect->y = (screenHeight - textureRect->h) / 2;
        break;

    case POS_BOTTOMLEFT:
    case POS_BOTTOMCENTER:
    case POS_BOTTOMRIGHT:
        textureRect->y = screenHeight - textureRect->h - globalMargins.h;
        break;

    default:
        textureRect->y = globalMargins.y; // Default to top if position not specified
        break;
    }
}


bool strendswith(const char *str, const char *suffix)
{
    if (!str || !suffix)
        return 0;

    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);

    if (lensuffix > lenstr)
        return 0;

    return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}


bool strcaseendswith(const char *str, const char *suffix)
{
    if (!str || !suffix)
        return 0;

    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);

    if (lensuffix > lenstr)
        return 0;

    return strncasecmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}


bool strstartswith(const char *str, const char *prefix)
{
    return strncmp(prefix, str, strlen(prefix)) == 0;
}


bool strcasestartswith(const char *str, const char *prefix)
{
    return strncasecmp(prefix, str, strlen(prefix)) == 0;
}


int strcasecmp(const char *s1, const char *s2)
{
    while (*s1 && *s2)
    {
        char c1 = tolower(*s1);
        char c2 = tolower(*s2);

        if (c1 != c2)
        {
            return c1 - c2;
        }

        s1++;
        s2++;
    }

    return *s1 - *s2;
}


int strncasecmp(const char *s1, const char *s2, size_t n)
{
    for (size_t i = 0; i < n; i++)
    {
        char c1 = tolower(s1[i]);
        char c2 = tolower(s2[i]);

        if (c1 != c2 || c1 == '\0' || c2 == '\0')
        {
            return c1 - c2;
        }
    }

    return 0;
}
