#include "macro.h"

char *next_item(char *s) {
    int jump = 0;
    for (; s; s++) {
        if (jump == 0 && isspace(*s)) {
            *s = 0; /*isolate first item*/
            jump = 1;
        } else if (jump > 0) {
            if (isspace(*s)) {
                jump++;
            } else {
                break;
            }
        }
    }
    return s;
}

const char *handle_key(char *s) {
#define UNDERSCORE '_'
#define SPACE ' '
    char *e, *d;
    d = 0;
    for (e = s; e && !isspace(*e); e++) {
        if (*e == UNDERSCORE) {
            d = e;
        }
    }
    *s = toupper(*s);
    if (d && d[1]) {
        d[1] = toupper(d[1]);
        memcpy(d, d + 1, e - d - 1);
        e[-1] = SPACE;
    }
    return s;
}

const char *handle_type(char *s) {
    char c = tolower(*s);
    if (c == 'b') { /*bigint*/
        return "int64";
    } else if (c == 'i') { /*int*/
        s = next_item(s);
        if (s && *s == 'u') { /*unsigned*/
            return "uint32";
        }
        return "int32";
    } else if (c == 'd' || c == 't') { /*data or timestamp*/
        return "time.Time";
    } else if (c == 'c' || c == 'v') { /*char or varchar*/
        return "string";
    }
    return "interface{}"; /*unknown type*/
}

void handle_line(char *s) {
    const char *key, *type;
    key = handle_key(s);
    s = next_item(s);
    type = handle_type(s);
    LOG("%s %s", key, type);
}

MAIN() {
    char *line = 0, *s;
    size_t n = 0;
    while (getline(&line, &n, stdin) != -1) {
        for (s = line; isspace(*s); s++) {} /*skip leading spaces*/
        handle_line(s);
    }
    free(line);
    return 0;
}