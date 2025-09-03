#include "global.h"
#include "../../../../actor/_custom-1.0/holText.h"

int GetStringCenterX(char* string, float scale)
{
    int len = GetTextPxWidth(string, scale);
    return (SCREEN_WIDTH / 2) - len / 2;
}