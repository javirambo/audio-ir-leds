#include "IRremote.h"

#define LR 9
#define LG 5
#define LB 6
#define AUDIO A0
#define RECV_PIN 2

#define log(a, b)          \
    {                      \
        Serial.print(a);   \
        Serial.print(':'); \
        Serial.println(b); \
    }

#define log_rgb()            \
    {                        \
        Serial.print('(');   \
        Serial.print(r);     \
        Serial.print(',');   \
        Serial.print(g);     \
        Serial.print(',');   \
        Serial.print(b);     \
        Serial.println(")"); \
    }

#define RGB(red, green, blue) \
    {                         \
        r = red;              \
        g = green;            \
        b = blue;             \
    }

#define RemoteCodesSize (sizeof(RemoteCodes) / sizeof(CodeFunc))
#define STEP_BRILLO 20

struct CodeFunc
{
    word code;     // codigo de la tecla pulsada del remoto.
    void (*run)(); // funcion a ejecutar cuando se pulsa la tecla.
};

IRrecv irrecv(RECV_PIN);
decode_results results;
int r = 0, g = 0, b = 0;
int rs = 1, gs = 1, bs = 1; // sentido de la onda para los smooth
bool is_flashing = false;
bool is_smoothing = false;
bool is_strobe = false;
bool is_w = false;
bool flash = false;
float hue = 0;

/* 
teclas del control remoto:
==================================================
  BRILLO_ARRIBA, BRILLO_ABAJO, OFF,       ON,
  ROJO,          VERDE,        VIOLETA,   W,
  ROJO_CLARO,    VERDE_CLARO,  VIOLETITA, FLASH,
  NARANJON,      CELESTE,      MAGENTA,   STROBE,
  NARANJA,       AZULITO,      LILA,      FADE,
  AMARILLO,      AZUL,         ROSA,      SMOOTH
==================================================
*/

// https://codegolf.stackexchange.com/questions/150250/hsl-to-rgb-values
/*#define S(o, n) r[t[int(h[0]) / 60 * 3 + o] + o - 2] = (n + h[2] - c / 2) * 255;
// parametros: float hsl[3] y int rgb[3]
void HSL2RGB(float *h, int *r)
{
    float g = 2 * h[2] - 1, c = (g < 0 ? 1 + g : 1 - g) * h[1], a = int(h[0]) % 120 / 60.f - 1;
    int t[] = {2, 2, 2, 3, 1, 2, 3, 3, 0, 4, 2, 0, 4, 1, 1, 2, 3, 1};
    S(0, c)
    S(1, c * (a < 0 ? 1 + a : 1 - a))
    S(2, 0)
}*/

#define S2(o, n) RGB[t[int(hue) / 60 * 3 + o] + o - 2] = (n + ele - c / 2) * 255;
void HSL_to_RGB(float hue, float ese, float ele){
    float g = 2 * ele - 1, c = (g < 0 ? 1 + g : 1 - g) * ese, a = int(hue) % 120 / 60.f - 1;
    int t[] = {2, 2, 2, 3, 1, 2, 3, 3, 0, 4, 2, 0, 4, 1, 1, 2, 3, 1};
    int RGB[3];
    S2(0, c);
    S2(1, c * (a < 0 ? 1 + a : 1 - a));
    S2(2, 0);
    r = (byte)RGB[0];
    g = (byte)RGB[1];
    b = (byte)RGB[2];
}

void shiftHueColor(unsigned int timeout)
{
    static unsigned long time1 = millis();
    if (millis() - time1 > timeout)
    {
        time1 = millis();
        hue = hue + 0.5;
        if (hue >= 360.0)
            hue = 0.0;
    }
}

//-- enciende las luces según el valor de tensión que ingresa por A0:
void audiorritmico()
{
    // el tono de color va rotando de a poquito...
    shiftHueColor(7);

    int analog = analogRead(AUDIO);
    float val = (float)analog / 1024.0;

  //  float HSL[3] = {hue, 1, val};
  //  int RGB[3]; // resultado
    HSL_to_RGB(hue, 1, val);
  //  r = (byte)RGB[0];
  //  g = (byte)RGB[1];
  //  b = (byte)RGB[2];

    analogWrite(LR, r);
    analogWrite(LG, g);
    analogWrite(LB, b);
}

void refreshLeds()
{
    if (is_w)
    {
        audiorritmico();
    }
    else
    {
        if (is_smoothing)
        {
            shiftHueColor(5);
            //--solo actua el cambio cada N milisegundos:
          //  if (t1 > 5)
           // {
              //  time1 = millis();
              //  hue = hue + 0.5;
             //   if (hue >= 360.0)
             //       hue = 0;
               // float HSL[3] = {hue, 1.0, 0.5};
              //  int RGB[3]; // resultado
                HSL_to_RGB(hue, 1.0, 0.5);
             //   r = (byte)RGB[0];
             //   g = (byte)RGB[1];
             //   b = (byte)RGB[2];
           // }
        }

        // control de saturacion:
        r = (r > 255) ? 255 : ((r < 0) ? 0 : r);
        g = (g > 255) ? 255 : ((g < 0) ? 0 : g);
        b = (b > 255) ? 255 : ((b < 0) ? 0 : b);

        //int t2 = millis() - time2;
        static unsigned long time2 = millis();
        if (millis() - time2 > 111)
        {
            time2 = millis();
            flash = !flash;
        }

        if ((is_flashing || is_strobe) && flash)
        {
            //el strobe es un flash con ROJO!
            analogWrite(LR, is_strobe ? 255 : 0);
            analogWrite(LG, 0);
            analogWrite(LB, 0);
        }
        else
        {
            analogWrite(LR, r);
            analogWrite(LG, g);
            analogWrite(LB, b);
        }
    }
}

void rojo() { RGB(255, 0, 0); }
void verde() { RGB(0, 255, 0); }
void violeta() { RGB(150, 22, 255); }
void rojo_claro() { RGB(255, 58, 58); }
void verde_claro() { RGB(38, 255, 139); }
void violetita() { RGB(116, 37, 252); }
void naranjon() { RGB(255, 110, 0); }
void celeste() { RGB(0, 203, 255); }
void magenta() { RGB(255, 0, 255); }
void naranja() { RGB(255, 153, 0); }
void azulito() { RGB(0, 117, 175); }
void lila() { RGB(209, 117, 255); }
void amarillo() { RGB(255, 246, 0); }
void azul() { RGB(0, 0, 255); }
void rosa() { RGB(255, 219, 232); }
void on() { RGB(255, 255, 255); }

void off()
{
    RGB(0, 0, 0);
    is_flashing = false;
    is_smoothing = false;
}

void brillo_arriba()
{
    r += STEP_BRILLO;
    g += STEP_BRILLO;
    b += STEP_BRILLO;
}

void brillo_abajo()
{
    r -= STEP_BRILLO;
    g -= STEP_BRILLO;
    b -= STEP_BRILLO;
}

//--se enciende el audioritmico:
void w()
{
    is_flashing = false;
    is_smoothing = false;
    is_strobe = false;
    is_w = true;
}

void strobe()
{
    //apago el flash y el smooth:
    is_flashing = false;
    is_smoothing = false;
    is_strobe = true;
    is_w = false;
    //strobe cambia de rojo a azul:
    azul();
}

void flashing()
{
    //apago el strobe:
    is_strobe = false;
    is_w = false;
    is_flashing = !is_flashing;
    //no puedo flashear el negro...
    if (r == 0 && g == 0 && b == 0 && is_flashing)
        on();
}

//--hace unas ondas de cambio de color...
//--depende del color seleccionado previamente.
//--si esta en flashing sigue el smooth.
void smooth()
{
    is_smoothing = !is_smoothing;
    //no puedo arrancar del negro...
    if (r == 0 && g == 0 && b == 0 && is_smoothing)
    {
        r = 0;
        g = 85;
        b = 170;
    }
}

//--se apaga lentamente...
void fade()
{
    while (r > 0 || g > 0 || b > 0)
    {
        r--;
        g--;
        b--;
        refreshLeds();
        delay(2);
    }
}

//#define TINKERCAD_CODES
#ifdef TINKERCAD_CODES
// TinkercadRemoteCodes
CodeFunc RemoteCodes[] = {
    {0x00FF, on},
    {0x807F, off},
    {0x40BF, amarillo},
    {0x20DF, azul},
    {0xA05F, rosa},
    {0x609F, lila},
    {0x10EF, azulito},
    {0x906F, naranja},
    {0x50AF, magenta},
    {0x30CF, celeste},
    {0xB04F, naranjon},
    {0x708F, violetita},
    {0x08F7, verde_claro},
    {0x8877, rojo_claro},
    {0x48B7, w},
    {0x28D7, strobe},
    {0xA857, fade},
    {0x6897, smooth},
    {0x18E7, flashing},
    {0x9867, brillo_abajo},
    {0x58A7, brillo_arriba}};

#else

// mapeo de códigos del IR y mi programa de luces:
CodeFunc RemoteCodes[] = {
    {0x705D, brillo_arriba},
    {0xC861, brillo_abajo},
    {0x0381, off},
    {0x72FD, on},
    {0x52E1, rojo},
    {0xA4DD, verde},
    {0x2345, violeta},
    {0x92C1, w},
    {0x9A81, rojo_claro},
    {0xEC7D, verde_claro},
    {0x009D, violetita},
    {0x5BE1, flashing},
    {0x4BBD, naranjon},
    {0x9DB9, celeste},
    {0x1C21, magenta},
    {0x8B9D, strobe},
    {0xB3E1, naranja},
    {0x05DD, azulito},
    {0x4445, lila},
    {0x7541, fade},
    {0x5E1D, amarillo},
    {0xB621, azul},
    {0x2E81, rosa},
    {0x5F7D, smooth}};
#endif

void setup()
{
    Serial.begin(9600);
    pinMode(13, OUTPUT);
    pinMode(LR, OUTPUT);
    pinMode(LG, OUTPUT);
    pinMode(LB, OUTPUT);
    pinMode(AUDIO, INPUT);
    irrecv.enableIRIn();
    Serial.println("RGB-IR OK");
}

// Dumps out the decode_results structure.
// Call this after IRrecv::decode()
void dump()
{
    if (results.bits == 0)
        return;
    Serial.print(results.bits, DEC);
    Serial.print(", ");
    Serial.println(results.value, HEX);
}

// determino cual programa correr dependiendo del código que llegó del control remoto.
void readRemoteControl()
{
    if (irrecv.decode(&results))
    {
        //para ver los códigos del remoto:
        //dump();
        for (unsigned int i = 0; i < RemoteCodesSize; i++)
        {
            if ((results.value & 0xffff) == RemoteCodes[i].code)
            {
                //--se ejecuta la funcion asociada a esa teacla o codigo:
                RemoteCodes[i].run();
                break;
            }
        }
        irrecv.resume(); // empezamos una nueva recepción
    }
}

void loop()
{
    readRemoteControl();
    refreshLeds();
}
