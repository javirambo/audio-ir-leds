#include "IRremote.h"

#define LR 3
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
unsigned long time1 = millis();
unsigned long time2 = millis();

/* 
lista de teclas del remoto:
===========================
  BRILLO_ARRIBA,
  BRILLO_ABAJO,
  OFF,
  ON,
  ROJO,
  VERDE,
  VIOLETA,
  W,
  ROJO_CLARO,
  VERDE_CLARO,
  VIOLETITA,
  FLASH,
  NARANJON,
  CELESTE,
  MAGENTA,
  STROBE,
  NARANJA,
  AZULITO,
  LILA,
  FADE,
  AMARILLO,
  AZUL,
  ROSA,
  SMOOTH
*/

void refreshLeds()
{
    int t1 = millis() - time1;
    int t2 = millis() - time2;
    if (is_smoothing)
    {
        //--solo actua el cambio cada N milisegundos:
        if (t1 > 5)
        {
            time1 = millis();
            r += rs;
            g += gs;
            b += bs;
            if (r > 255)
                rs = -1;
            if (g > 255)
                gs = -1;
            if (b > 255)
                bs = -1;
            if (r < 0)
                rs = 1;
            if (g < 0)
                gs = 1;
            if (b < 0)
                bs = 1;
        }
    }

    // control de saturacion:
    r = (r > 255) ? 255 : ((r < 0) ? 0 : r);
    g = (g > 255) ? 255 : ((g < 0) ? 0 : g);
    b = (b > 255) ? 255 : ((b < 0) ? 0 : b);

    if (t2 > 500)
    {
        time2 = millis();
        flash = !flash;
    }

    //--refresh leds:

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

void w()
{
    is_w = !is_w;
}

void strobe()
{
    //apago el flash y el smooth:
    is_flashing = false;
    is_smoothing = false;
    is_strobe = true;
    //strobe cambia de rojo a azul:
    azul();
}

void flashing()
{
    //apago el strobe:
    is_strobe = false;
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
        violetita();
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

// mapeo de códigos del IR y mi programa de luces:
/*unsigned int RemoteCodes55[] = {
    0x705D, BRILLO_ARRIBA,
    0xC861, BRILLO_ABAJO,
    0x0381, OFF,
    0x72FD, ON,
    0x52E1, ROJO,
    0xA4DD, VERDE,
    0x2345, VIOLETA,
    0x92C1, W,
    0x9A81, ROJO_CLARO,
    0xEC7D, VERDE_CLARO,
    0x009D, VIOLETITA,
    0x5BE1, FLASH,
    0x4BBD, NARANJON,
    0x9DB9, CELESTE,
    0x1C21, MAGENTA,
    0x8B9D, STROBE,
    0xB3E1, NARANJA,
    0x05DD, AZULITO,
    0x4445, LILA,
    0x7541, FADE,
    0x5E1D, AMARILLO,
    0xB621, AZUL,
    0x2E81, ROSA,
    0x5F7D, SMOOTH};
*/

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
        for (int i = 0; i < RemoteCodesSize; i++)
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
