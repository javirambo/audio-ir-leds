
/*
    https://www.programmingalgorithms.com/algorithm/hsl-to-rgb/

    H[0,360]  S[0,1]  L[0,1]
    HSL data = new HSL(138.0, 0.50f, 0.76f);
    RGB value = HSLToRGB(data);
*/

class RGB
{
public:
    unsigned char R;
    unsigned char G;
    unsigned char B;

    RGB() : R(0), G(0), B(0) {}

    RGB(unsigned char r, unsigned char g, unsigned char b)
    {
        R = r;
        G = g;
        B = b;
    }

    bool Equals(RGB rgb) { return (R == rgb.R) && (G == rgb.G) && (B == rgb.B); }
};

class HSL
{
public:
    float H;
    float S;
    float L;

    HSL() : H(0), S(0), L(0) {}

    HSL(float h, float s, float l)
    {
        H = h;
        S = s;
        L = l;
    }

    bool Equals(HSL hsl) { return (H == hsl.H) && (S == hsl.S) && (L == hsl.L); }
};

float HueToRGB(float v1, float v2, float vH);
RGB HSLToRGB(HSL hsl);
