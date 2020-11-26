/**
 * ESTA VERSION V3 ESTA TERMINADA.
 * -EL SMOOTH SI SE PRENDE VA BARRIENDO LOS COLORES EN CUALQUIER MOMENTO,
 *  POR EJ, EN EL FLASH, STROBE, AUDIORRITMICO.
 * -EL STROBE ES UN FLASH DE ROJO A COLOR (EL QUE SE SELECCIONE)
 * -EL FLASH ES NEGRO A COLOR.
 * -EL FADE APAGA TODO DE A POCO.
 * 
 * PROBLEMAS:
 * BRILLO ARRIBA / ABAJO PUEDE SATURARSE Y PIERDE EL COLOR ORIGINAL.
 * 
 * JAVIER RAMBALDO
 * JUNIO DE 2020
 */
#include "IRremote.h"

//#define TINKERCAD_CODES
#ifdef TINKERCAD_CODES
#define FLASH_TIME 500
#define STEP_BRILLO 50
#define TIME_SHIFT_COLOR 15
#else
#define FLASH_TIME 45
#define STEP_BRILLO 15
#define TIME_SHIFT_COLOR 15
#endif

#define LR 9
#define LG 5
#define LB 6
#define AUDIO A0
#define RECV_PIN 2

#define RGB(red, green, blue) \
    {                         \
        r = red;              \
        g = green;            \
        b = blue;             \
    }
#define RemoteCodesSize (int)(sizeof(RemoteCodes) / sizeof(CodeFunc))

struct CodeFunc
{
    word code;     // codigo de la tecla pulsada del remoto.
    void (*run)(); // funcion a ejecutar cuando se pulsa la tecla.
};

IRrecv irrecv(RECV_PIN);
decode_results results;
int r = 0, g = 0, b = 0;
bool is_flashing = false;
bool is_smoothing = false;
bool is_strobe = false;
bool is_w = false;
int hue = 0;

// HSL color values are specified with: hsl(hue, saturation, lightness).
// https://codegolf.stackexchange.com/questions/150250/hsl-to-rgb-values
#define S2(o, n) RGB[matrix[hue / 60 * 3 + o] + o - 2] = byte((n + lightness - c / 2) * 255);
void HSL_to_RGB(int hue, float saturation, float lightness)
{
    static const int matrix[] = {2, 2, 2, 3, 1, 2, 3, 3, 0, 4, 2, 0, 4, 1, 1, 2, 3, 1};
    float g = 2 * lightness - 1;
    float c = (g < 0 ? 1 + g : 1 - g) * saturation;
    float a = (hue % 120) / 60.0 - 1;
    byte RGB[3];
    S2(0, c);
    S2(1, c * (a < 0 ? 1 + a : 1 - a));
    S2(2, 0);
    r = RGB[0];
    g = RGB[1];
    b = RGB[2];
}

void shiftHueColor()
{
    static unsigned long time1 = millis();
    if (millis() - time1 > TIME_SHIFT_COLOR)
    {
        time1 = millis();
        if (++hue >= 360)
            hue = 0;
    }
}

// intercambia variable ON/OFF para prender o apagar el flash.
bool flash()
{
    static bool flash = false;
    static unsigned long time2 = millis();
    if (millis() - time2 > FLASH_TIME)
    {
        time2 = millis();
        flash = !flash;
    }
    return flash;
}

void refreshLeds()
{
    // el tono de color va rotando de a poquito...
    if (is_smoothing)
        shiftHueColor();

    if (is_w)
    {
        //-- audiorritmico: enciende las luces según el valor de tensión que ingresa por A0:
        //-- subo solo el brillo, el tono lo da el HUE global.
        HSL_to_RGB(hue, 1.0, (float)analogRead(AUDIO) / 1024.0);
    }
    else if (is_smoothing)
    {
        //-- solo barre los todos de color:
        HSL_to_RGB(hue, 1.0, 0.5);
    }

    if ((is_flashing || is_strobe) && flash())
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
void blanco() { RGB(255, 255, 255); }
void negro() { RGB(0, 0, 0); }

void off()
{
    negro();
    is_flashing = false;
    is_smoothing = false;
    is_strobe = false;
    is_w = false;
}

void brillo_arriba()
{
    r += STEP_BRILLO;
    g += STEP_BRILLO;
    b += STEP_BRILLO;
    // control de saturacion:
    r = (r > 255) ? 255 : ((r < 0) ? 0 : r);
    g = (g > 255) ? 255 : ((g < 0) ? 0 : g);
    b = (b > 255) ? 255 : ((b < 0) ? 0 : b);
}

void brillo_abajo()
{
    r -= STEP_BRILLO;
    g -= STEP_BRILLO;
    b -= STEP_BRILLO;
    // control de saturacion:
    r = (r > 255) ? 255 : ((r < 0) ? 0 : r);
    g = (g > 255) ? 255 : ((g < 0) ? 0 : g);
    b = (b > 255) ? 255 : ((b < 0) ? 0 : b);
}

//--se enciende el audioritmico:
void w()
{
    is_flashing = false;
    is_strobe = false;
    is_w = true;
}

//--flash pero en lugar de negro es rojo! el otro color es el que está (o se puede cambiar)
void strobe()
{
    //apago el flash y el smooth:
    is_flashing = false;
    is_strobe = !is_strobe;
    is_w = false;
    //strobe cambia de rojo a azul:
    azul();
}

//--prende apaga con el color seleccionado (hasta con smooth)
void flashing()
{
    //apago el strobe:
    is_strobe = false;
    is_w = false;
    is_flashing = !is_flashing;
    //no puedo flashear el negro...
    if (r < 10 && g < 10 && b < 10 && is_flashing)
        blanco();
}

//--hace unas ondas de cambio de color...
//--depende del color seleccionado previamente.
//--si esta en flashing sigue el smooth.
void smooth()
{
    is_smoothing = !is_smoothing;
    //no puedo arrancar del negro...prendo cualquier color con brillo max.
    if (is_smoothing)
        HSL_to_RGB(0, 1, 1);
}

//--se apaga lentamente...
void fade()
{
    while (r > 0 || g > 0 || b > 0)
    {
        if (r > 0)
            r--;
        if (g > 0)
            g--;
        if (b > 0)
            b--;
        analogWrite(LR, r);
        analogWrite(LG, g);
        analogWrite(LB, b);
        delay(5);
    }
    off();
}

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

#ifdef TINKERCAD_CODES
// TinkercadRemoteCodes
CodeFunc RemoteCodes[] = {
    {0x00FF, blanco},
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
    {0x72FD, blanco},
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
    pinMode(13, OUTPUT);
    pinMode(LR, OUTPUT);
    pinMode(LG, OUTPUT);
    pinMode(LB, OUTPUT);
    pinMode(AUDIO, INPUT);
    irrecv.enableIRIn();
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
