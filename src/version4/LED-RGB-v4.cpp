/**
 * VERSION 4
 * cambios desde la V3:
 * +cambie todos los colores a HSL...no va mas el RGB.
 * 
 * -EL SMOOTH SI SE PRENDE VA BARRIENDO LOS COLORES EN CUALQUIER MOMENTO,
 *  POR EJ, EN EL FLASH, STROBE, AUDIORRITMICO.
 * -EL STROBE ES UN FLASH DE ROJO A COLOR (EL QUE SE SELECCIONE)
 * -EL FLASH ES NEGRO A COLOR.
 * -EL FADE APAGA TODO DE A POCO.
 * 
 * JAVIER RAMBALDO
 * JUNIO DE 2020
 */
#include "IRremote.h"

#define log(a, b)          \
    {                      \
        Serial.print(a);   \
        Serial.print(':'); \
        Serial.println(b); \
    }
#define logRGB()             \
    {                        \
        Serial.print('(');   \
        Serial.print((int)(rgb[0] * 255));     \
        Serial.print(',');   \
        Serial.print((int)(rgb[1] * 255));     \
        Serial.print(',');   \
        Serial.print((int)(rgb[2] * 255));     \
        Serial.println(')'); \
    }
#define logHSL()                  \
    {                             \
        Serial.print('(');        \
        Serial.print(hue);        \
        Serial.print(',');        \
        Serial.print(saturation); \
        Serial.print(',');        \
        Serial.print(brightness); \
        Serial.println(')');      \
    }

#define TINKERCAD_CODES
#ifdef TINKERCAD_CODES
#define FLASH_TIME 500
#define STEP_BRILLO 0.1
#define TIME_SHIFT_COLOR 15
#define VELOCIDAD_MINIMA 5
#define VELOCIDAD_NORMAL 10
#define VELOCIDAD_MAXIMA 20
#else
#define FLASH_TIME 45
#define STEP_BRILLO 15
#define TIME_SHIFT_COLOR 15
#define VELOCIDAD_MINIMA 5
#define VELOCIDAD_NORMAL 10
#define VELOCIDAD_MAXIMA 20
#endif

#define LedR 9
#define LedG 5
#define LedB 6
#define AUDIO A0
#define RECV_PIN 2

#define RemoteCodesSize (int)(sizeof(RemoteCodes) / sizeof(CodeFunc))

// HSL: HUE [0-360], S [0-100%] L [0-100%]
//#define HSL(h, s, l) hsv2rgb(h / 360.0, s / 100.0, l / 100.0)
#define HSL(h, s, l)            \
    {                           \
        hue = h / 360.0;        \
        saturation = s / 100.0; \
        brightness = l / 100.0; \
    }

struct CodeFunc
{
    word code;     // codigo de la tecla pulsada del remoto.
    void (*run)(); // funcion a ejecutar cuando se pulsa la tecla.
};

IRrecv irrecv(RECV_PIN);
decode_results results;

bool is_w_on;
bool is_flash_on;
bool is_strobe_on;
bool is_fade_on;
bool is_smooth_on;

float hue;        // tono de rojo a rojo
float saturation; //  del gris al color puro.
float brightness; //  del negro al blanco (pasando por el color)
float rgb[3];
unsigned int velocidad;

// https://gist.github.com/postspectacular/2a4a8db092011c6743a7
// HSV->RGB conversion based on GLSL version
// expects hsv channels defined in 0.0 .. 1.0 interval

float fract(float x) { return x - int(x); }
float mix(float a, float b, float t) { return a + (b - a) * t; }

void hsv2rgb(float h, float s, float b)
{
    // h = hue; s = saturation; b = brightness
    rgb[0] = s * mix(1.0, constrain(abs(fract(h + 1.0) * 6.0 - 3.0) - 1.0, 0.0, 1.0), b);
    rgb[1] = s * mix(1.0, constrain(abs(fract(h + 0.6666666) * 6.0 - 3.0) - 1.0, 0.0, 1.0), b);
    rgb[2] = s * mix(1.0, constrain(abs(fract(h + 0.3333333) * 6.0 - 3.0) - 1.0, 0.0, 1.0), b);
}

// enciende los leds segun el tono brillo etc.
void show_color()
{
    hsv2rgb(hue, saturation, brightness);
    logRGB();
    logHSL();
    analogWrite(LedR, (int)(rgb[0] * 255));
    analogWrite(LedG, (int)(rgb[1] * 255));
    analogWrite(LedB, (int)(rgb[2] * 255));
}

void shift_hue_color()
{
    static unsigned long time1 = millis();
    if (millis() - time1 > TIME_SHIFT_COLOR)
    {
        time1 = millis();
        hue += 0.01;
        if (hue >= 1.0)
            hue = 0.0;
    }
}

void refreshLeds()
{
    if (is_w_on)
    {
        //-- audiorritmico: enciende las luces según el valor de tensión que ingresa por A0:
        //-- subo solo el brillo, el tono lo da el HUE global.
        brightness = (float)analogRead(AUDIO) / 1024.0;
    }
    else if (is_strobe_on || is_flash_on)
    {
        static bool flash = false;
        static unsigned long time2 = millis();
        if (millis() - time2 > velocidad)
        {
            time2 = millis();
            flash = !flash;
            if (is_strobe_on)
            {
                //es un flash pero con color complementario en lugar de negro:
                hue = 1 - hue;
            }
            else
            {
                //prende/apaga el flash.
                brightness = flash ? 0 : 1;
            }
        }
    }
    else if (is_fade_on)
    {
        // es un flash pero suave (fade in/out)
        static unsigned long time = 0;
        static bool sube = false;
        if (millis() - time > 5)
        {
            time = millis();
            if (sube)
            {
                brightness += 0.05;
                if (brightness >= 1)
                    sube = false;
            }
            else
            {
                brightness -= 0.05;
                if (brightness <= 0)
                    sube = true;
            }
        }
    }
    show_color();
}

void rojo() { HSL(0, 100, 50); }
void verde() { HSL(120, 255, 0); }
void violeta() { HSL(264, 100, 27); }
void rojo_claro() { HSL(2, 100, 57); }
void verde_claro() { HSL(142, 100, 38); }
void violetita() { HSL(254, 57, 48); }
void naranjon() { HSL(15, 100, 49); }
void celeste() { HSL(192, 100, 50); }
void magenta() { HSL(255, 0, 255); }
void naranja() { HSL(276, 100, 50); }
void azulito() { HSL(217, 78, 38); }
void lila() { HSL(276, 45, 52); }
void amarillo() { HSL(50, 100, 50); }
void azul() { HSL(0, 0, 255); }
void rosa() { HSL(320, 100, 67); }
void blanco() { HSL(0, 0, 100); }
void negro() { HSL(0, 0, 0); }

void off()
{
    negro();
    velocidad = VELOCIDAD_NORMAL;
    is_w_on = false;
    is_flash_on = false;
    is_strobe_on = false;
    is_fade_on = false;
    is_smooth_on = false;
}

void brillo_arriba()
{
    if (is_flash_on || is_strobe_on)
    {
        if (++velocidad > VELOCIDAD_MAXIMA)
            velocidad = VELOCIDAD_MAXIMA;
    }
    else
        brightness += STEP_BRILLO;
}

void brillo_abajo()
{
    if (is_flash_on || is_strobe_on)
    {
        if (--velocidad < VELOCIDAD_MINIMA)
            velocidad = VELOCIDAD_MINIMA;
    }
    else
        brightness -= STEP_BRILLO;
}

//--se enciende el audioritmico:
void w()
{
    is_w_on = !is_w_on;
    is_flash_on = false;
    is_strobe_on = false;
    is_fade_on = false;
}

//--flash pero en lugar de negro es el color complementario
void strobe()
{
    is_w_on = false;
    is_flash_on = false;
    is_strobe_on = !is_strobe_on;
    is_fade_on = false;
}

//--prende apaga con el color actual (hasta con smooth)
void flashing()
{
    is_w_on = false;
    is_flash_on = !is_flash_on;
    is_strobe_on = false;
    is_fade_on = false;
}

// enciende el ciclo de color, sino queda en el ultimo color mostrado.
void smooth()
{
    is_smooth_on = !is_smooth_on;
}

// flash pero con fade in/out
void fade()
{
    is_w_on = false;
    is_flash_on = false;
    is_strobe_on = false;
    is_fade_on = !is_flash_on;
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
    {0x807F, rojo},
    {0x40BF, verde},
    {0x20DF, violeta},
    {0xA05F, rosa},
    {0x609F, rojo_claro},
    {0x10EF, verde_claro},
    {0x906F, violetita},
    {0x50AF, naranjon},
    {0x30CF, celeste},
    {0xB04F, magenta},
    {0x708F, naranja},
    {0x08F7, azulito},
    {0x8877, lila},
    {0x48B7, amarillo},
    {0x28D7, azul},
    {0xA857, rosa},
    {0x6897, off},
    {0x18E7, off},
    {0x9867, off},
    {0x58A7, off}};
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
    Serial.begin(9600);
    pinMode(13, OUTPUT);
    pinMode(LedR, OUTPUT);
    pinMode(LedG, OUTPUT);
    pinMode(LedB, OUTPUT);
    pinMode(AUDIO, INPUT);
    irrecv.enableIRIn();
    off();
}

// determino cual programa correr dependiendo del código que llegó del control remoto.
void readRemoteControl()
{
    if (irrecv.decode(&results))
    {
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

void itsAlive()
{
    static unsigned long timer = 0;
    digitalWrite(13, timer < 1000);
    if (++timer > 300000L)
        timer = 0;
}

void loop()
{
    readRemoteControl();

    // el tono de color va rotando de a poquito...
    if (is_smooth_on)
        shift_hue_color();

    refreshLeds();

    itsAlive();
}