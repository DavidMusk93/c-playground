#include "../../macro.h"

#define COLOR_PREFIX "\033["
#define COLOR_SUFFIX "m"

#define COLOR_YELLOW "33"
#define COLOR_BLINK  "5"
#define COLOR_RESET  "0"
// ref,https://stackoverflow.com/questions/17439482/how-to-make-a-text-blink-in-shell-script
#define COLOR_YELLOWBLINK COLOR_YELLOW ";" COLOR_BLINK
#define COLOR(x) COLOR_PREFIX COLOR_##x COLOR_SUFFIX

MAIN() {
    const char *prompt = R"(Hi, guys!
My name is Allen, I am happy to be here.
This
is
a
raw
string.)";
    const char *blink = COLOR(YELLOWBLINK) "TO BE CONTINUED!" COLOR(RESET);
    LOG("%s,%s\n", prompt, blink);
}