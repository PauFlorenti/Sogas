#include "entry.h"

int main()
{
    std::cout << "Hello Sogas!\n";
    SFATAL("This is a test %d right %s?", 45, "Pau");
    SERROR("This is a test %d right %s?", 45, "Pau");
    SWARNING("This is a test %d right %s?", 45, "Pau");
    SDEBUG("This is a test %d right %s?", 45, "Pau");
    STRACE("This is a test %d right %s?", 45, "Pau");

    return Sogas::GameEntry();
}