
/*
    https://www.programmingalgorithms.com/algorithm/hsl-to-rgb/

    H[0,360]  S[0,1]  L[0,1]
    HSL data = new HSL(138.0, 0.50f, 0.76f);
    RGB value = HSLToRGB(data);
*/
#include <Arduino.h>
#include "HSLToRGB.h"

float HueToRGB(float v1, float v2, float vH)
{
    if (vH < 0)
        vH += 1;

    if (vH > 1)
        vH -= 1;

    if ((6 * vH) < 1)
        return (v1 + (v2 - v1) * 6 * vH);

    if ((2 * vH) < 1)
        return v2;

    if ((3 * vH) < 2)
        return (v1 + (v2 - v1) * ((2.0f / 3.0) - vH) * 6.0);

    return v1;
}

RGB HSLToRGB(HSL hsl)
{
    unsigned char r = 0;
    unsigned char g = 0;
    unsigned char b = 0;

    if (hsl.S == 0)
    {
        r = g = b = (unsigned char)(hsl.L * 255);
    }
    else
    {
        float v1, v2;
        float hue = hsl.H / 360.0;

        v2 = (hsl.L < 0.5) ? (hsl.L * (1 + hsl.S)) : ((hsl.L + hsl.S) - (hsl.L * hsl.S));
        v1 = 2 * hsl.L - v2;

        r = (unsigned char)(255 * HueToRGB(v1, v2, hue + (1.0f / 3.0)));
        g = (unsigned char)(255 * HueToRGB(v1, v2, hue));
        b = (unsigned char)(255 * HueToRGB(v1, v2, hue - (1.0f / 3.0)));
    }

    return RGB(r, g, b);
}
