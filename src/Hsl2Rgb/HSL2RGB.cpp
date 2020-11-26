
// Given H,S,L in range of 0-1
// Returns a Color RGB in range of 0-255
unsigned long HSL2RGB(double h, double sl, double l)
{
    double v;
    double r, g, b;
    r = l; // default to gray
    g = l;
    b = l;
    v = (l <= 0.5) ? (l * (1.0 + sl)) : (l + sl - l * sl);
    if (v > 0)
    {
        double m;
        double sv;
        int sextant;
        double fract, vsf, mid1, mid2;
        m = l + l - v;
        sv = (v - m) / v;
        h *= 6.0;
        sextant = (int)h;
        fract = h - sextant;
        vsf = v * sv * fract;
        mid1 = m + vsf;
        mid2 = v - vsf;
        switch (sextant)
        {
        case 0:
            r = v;
            g = mid1;
            b = m;
            break;
        case 1:
            r = mid2;
            g = v;
            b = m;
            break;
        case 2:
            r = m;
            g = v;
            b = mid1;
            break;
        case 3:
            r = m;
            g = mid2;
            b = v;
            break;
        case 4:
            r = mid1;
            g = m;
            b = v;
            break;
        case 5:
            r = v;
            g = m;
            b = mid2;
            break;
        }
    }
    unsigned char rr = r * 255.0f;
    unsigned char gg = g * 255.0f;
    unsigned char bb = b * 255.0f;
    // retorna 24 bits RGB
    return bb | (gg << 8) || (rr << 16);
}

// https://codegolf.stackexchange.com/questions/150250/hsl-to-rgb-values
#define S(o, n) r[t[int(h[0]) / 60 * 3 + o] + o - 2] = (n + h[2] - c / 2) * 255;
//float hsl[3] y int rgb[3]
void C(float *h, int *r)
{
    float g = 2 * h[2] - 1, c = (g < 0 ? 1 + g : 1 - g) * h[1], a = int(h[0]) % 120 / 60.f - 1;
    int t[] = {2, 2, 2, 3, 1, 2, 3, 3, 0, 4, 2, 0, 4, 1, 1, 2, 3, 1};
    S(0, c)
    S(1, c * (a < 0 ? 1 + a : 1 - a)) S(2, 0)
}

/* otro

let h: CGFloat = 0
let s: CGFloat = 0
let l: CGFloat = 0

let result = f(h: h/360, s: s/100, l: l/100).map { Int(round($0*255)) }
print(result)
*/