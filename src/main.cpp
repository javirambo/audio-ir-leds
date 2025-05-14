/**
 * VERSION 6
 * cambios desde la V3:
 * +cambie todos los colores a HSV...no va mas el RGB.
 *
 * -EL SMOOTH SI SE PRENDE VA BARRIENDO LOS COLORES EN CUALQUIER MOMENTO,
 *  POR EJ, EN EL FLASH, STROBE, AUDIORRITMICO.
 * -EL STROBE ES UN FLASH DEL NEGRO AL COLOR SELECCIONADO.
 * -EL FLASH ES NEGRO A BLANCO.
 * -EL FADE ES UN FLASH SUAVE, DE NEGRO AL COLOR SELECCIONADO.
 * -LA TECLA W ES EL AUDIORRITMICO.
 *
 * -LAS FLECHAS ARRIBA / ABAJO CAMBIAN LA VELOCIDAD DE LOS FLASHES,
 *  PERO EN COLORES FIJOS CAMBIA EL BRILLO,
 *  (EN AUDIORRITMICO NO TIENE EFECTO, YA QUE EL BRILLO CAMBIA CON LA MUSICA)
 *
 * JAVIER RAMBALDO
 * NOVIEMBRE DE 2020
 */
#include "IRremote/IRremote.h"
#include "HSLToRGB.h"

#define STEP_BRILLO 0.033
#define BRILLO_MAXIMO 0.5
#define TIME_SHIFT_COLOR 15
#define STEP_VELOCIDAD 10
#define VELOCIDAD_MINIMA 10
#define VELOCIDAD_NORMAL 100
#define VELOCIDAD_MAXIMA 500

#define RemoteCodesSize (int)(sizeof(RemoteCodes) / sizeof(CodeFunc))

//-- configuracion de los pines del arduino.
const int LedR = 9;
const int LedG = 5;
const int LedB = 6;
const int AUDIO_IN = A0;
const int RECV_PIN = 2;

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
unsigned int velocidad;
HSL selected_color;

void logColor(float R, float G, float B, float H, float S, float V)
{
    static byte bR, bG, bB;
    if (R - bR + G - bG + B - bB != 0)
    {
        bR = R;
        bG = G;
        bB = B;
        Serial.print("RGB(");
        Serial.print(R);
        Serial.print(',');
        Serial.print(G);
        Serial.print(',');
        Serial.print(B);
        Serial.print(") selected_color = HSL(");
        Serial.print(H);
        Serial.print(',');
        Serial.print(S);
        Serial.print(',');
        Serial.print(V);
        Serial.println(')');
    }
}

float leerAudio()
{
    static float max, min, adc, top, val, percent;

    // leo el audio analogico
    adc = analogRead(A0);

    // guardo los picos o extremos:
    if (adc < min)
        min = adc;
    if (adc > max)
        max = adc;

    // control automático de ganancia:
    top = (max - min);
    val = (adc - min);
    percent = val / top;

    // compuerta de sonido: lo apaga si no hay sonido.
    if (top < 3)
    {
        percent = 0;
        digitalWrite(13, 1);
    }
    else
    {
        digitalWrite(13, 0);
    }

    // controlo saturacion: brillo va de 0 a 1
    if (percent < 0)
        percent = 0;
    if (percent > 1)
        percent = 1;

    // los extremos se van juntando o normalizando suavemente:
    min += 0.001;
    max -= 0.001;

    return percent;
}

void refreshLeds()
{
    // HSL color = HSL(selected_color.H, selected_color.S, selected_color.L);

    if (is_w_on)
    {
        //-- audiorritmico: enciende las luces según el valor de tensión que ingresa por A0:
        //-- subo solo el brillo, el tono lo da el HUE global.
        float nivel_audio = leerAudio();
        selected_color.L = nivel_audio;
    }
    else if (is_strobe_on || is_flash_on)
    {
        static bool flash = false;
        static unsigned long time_fade = 0;
        if (millis() - time_fade > velocidad)
        {
            time_fade = millis();
            flash = !flash;
            if (is_strobe_on)
            {
                // es un flash pero con el color seleccionado
                if (flash)
                    selected_color.L = 0.5;
                else
                    selected_color.L = 0;
            }
            else
            {
                // prende/apaga el flash.
                if (flash)
                {
                    selected_color = HSL(0, 0, 1); // blanco
                }
                else
                {
                    selected_color = HSL(0, 0, 0); // negro
                }
            }
        }
    }
    else if (is_fade_on)
    {
        // es un flash pero suave (fade in/out)
        static unsigned long time = 0;
        static bool sube = false;
        if (millis() - time > velocidad / 2.0)
        {
            time = millis();
            if (sube)
            {
                selected_color.L += 0.01;
                if (selected_color.L >= 0.5)
                {
                    sube = false;
                    selected_color.L = 0.5;
                }
            }
            else
            {
                selected_color.L -= 0.01;
                if (selected_color.L <= 0)
                {
                    sube = true;
                    selected_color.L = 0;
                }
            }
        }
    }

    // enciende los leds segun el tono brillo etc.
    // convierte el espacio de color HSL a RGB.

    RGB rgb = HSLToRGB(selected_color);
    analogWrite(LedR, rgb.R);
    analogWrite(LedG, rgb.G);
    analogWrite(LedB, rgb.B);
}

void dump()
{
    if (results.bits == 0)
        return;
    /* para generar la tabla:
    unsigned int v = results.value & 0xffff;
    Serial.print("{0x");
    if (v <= 0xff)
        Serial.print("00");
    Serial.print(v, HEX);
    Serial.println(", },");*/
    Serial.println(results.value, HEX);
}

void rojo()
{
    selected_color = HSL(0, 1, 0.5);
    Serial.println("ROJO");
}
void verde()
{
    selected_color = HSL(120, 1, 0.5);
    Serial.println("VERDE");
}
void violeta()
{
    selected_color = HSL(274, 1, 0.5);
    Serial.println("VIOLETA");
}
void rojo_claro()
{
    selected_color = HSL(343, 1, 0.5);
    Serial.println("ROJITO");
}
void verde_claro()
{
    selected_color = HSL(152, 1, 0.5);
    Serial.println("VERDITO");
}
void violetita()
{
    selected_color = HSL(285, 1, 0.6);
    Serial.println("VIOLETITA");
}
void naranjon()
{
    selected_color = HSL(16, 1, .46);
    Serial.println("NARANJON");
}
void celeste()
{
    selected_color = HSL(195, .8, .45);
    Serial.println("CELESTE");
}
void magenta()
{
    selected_color = HSL(340, 1, .4);
    Serial.println("MAGENTA");
}
void naranja()
{
    selected_color = HSL(36, .66, .5);
    Serial.println("NARANJA");
}
void azulito()
{
    selected_color = HSL(200, 1, .35);
    Serial.println("AZULITO");
}
void lila()
{
    selected_color = HSL(300, .80, .4);
    Serial.println("LILA");
}
void amarillo()
{
    selected_color = HSL(60, 1, .5);
    Serial.println("AMARILLO");
}
void azul()
{
    selected_color = HSL(240, 1, .5);
    Serial.println("AZUL");
}
void rosa()
{
    selected_color = HSL(315, .5, .5);
    Serial.println("ROSA");
}
void blanco()
{
    selected_color = HSL(0, 0, 1);
    Serial.println("BLANCO");
}
void negro()
{
    selected_color = HSL(0, 0, 0);
    Serial.println("NEGRO");
}

void off()
{
    negro();
    velocidad = VELOCIDAD_NORMAL;
    is_w_on = false;
    is_flash_on = false;
    is_strobe_on = false;
    is_fade_on = false;
    is_smooth_on = false;
    Serial.println("OFF");
}

void brillo_arriba()
{
    if (is_flash_on || is_strobe_on || is_fade_on)
    {
        if (velocidad > VELOCIDAD_MINIMA)
            velocidad -= STEP_VELOCIDAD;
        if (velocidad < VELOCIDAD_MINIMA)
            velocidad = VELOCIDAD_MINIMA;
        Serial.print("VELOCIDAD ^ ");
        Serial.println(velocidad);
    }
    else
    {
        if (selected_color.L < BRILLO_MAXIMO)
            selected_color.L += STEP_BRILLO;
        if (selected_color.L > BRILLO_MAXIMO)
            selected_color.L = BRILLO_MAXIMO;
        Serial.print("BRILLO ^ ");
        Serial.println(selected_color.L);
    }
}

void brillo_abajo()
{
    if (is_flash_on || is_strobe_on || is_fade_on)
    {
        if (velocidad < VELOCIDAD_MAXIMA)
            velocidad += STEP_VELOCIDAD;
        if (velocidad > VELOCIDAD_MAXIMA)
            velocidad = VELOCIDAD_MAXIMA;
        Serial.print("VELOCIDAD v ");
        Serial.println(velocidad);
    }
    else
    {
        if (selected_color.L > 0)
            selected_color.L -= STEP_BRILLO;
        if (selected_color.L < 0)
            selected_color.L = 0;
        Serial.print("BRILLO v ");
        Serial.println(selected_color.L);
    }
}

//--se enciende el audioritmico:
void w()
{
    is_w_on = !is_w_on;
    is_flash_on = false;
    is_strobe_on = false;
    is_fade_on = false;
    Serial.print("W (AUDIO) ");
    Serial.println(is_w_on ? "ON" : "OFF");
}

//--flash pero en lugar de negro es el color complementario
void strobe()
{
    is_w_on = false;
    is_flash_on = false;
    is_strobe_on = !is_strobe_on;
    is_fade_on = false;
    Serial.print("STROBE ");
    Serial.println(is_strobe_on ? "ON" : "OFF");
    if (!is_strobe_on)
        off();
}

//--prende apaga con el color actual (hasta con smooth)
void flashing()
{
    is_w_on = false;
    is_flash_on = !is_flash_on;
    is_strobe_on = false;
    is_fade_on = false;
    Serial.print("FLASH ");
    Serial.println(is_flash_on ? "ON" : "OFF");
    if (!is_flash_on)
        off();
}

// enciende el ciclo de color, sino queda en el ultimo color mostrado.
void smooth()
{
    is_smooth_on = !is_smooth_on;
    Serial.print("SMOOTH ");
    Serial.println(is_smooth_on ? "ON" : "OFF");
}

// flash pero con fade in/out
void fade()
{
    is_w_on = false;
    is_flash_on = false;
    is_strobe_on = false;
    is_fade_on = !is_fade_on;
    Serial.print("FADE ");
    Serial.println(is_fade_on ? "ON" : "OFF");
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
// mapeo de códigos del IR y mi programa de luces:
CodeFunc RemoteCodes[] = {
    {0x00FF, brillo_arriba},
    {0x807F, brillo_abajo},
    {0x40BF, off},
    {0xC03F, blanco},
    {0x20DF, rojo}, //
    {0xA05F, verde},
    {0x609F, violeta},
    {0xE01F, w},
    {0x10EF, rojo_claro}, //
    {0x906F, verde_claro},
    {0x50AF, violetita},
    {0xD02F, flashing},
    {0x30CF, naranjon}, //
    {0xB04F, celeste},
    {0x708F, magenta},
    {0xF00F, strobe},
    {0x08F7, naranja}, //
    {0x8877, azulito},
    {0x48B7, lila},
    {0xC837, fade},
    {0x28D7, amarillo}, //
    {0xA857, azul},
    {0x6897, rosa},
    {0xE817, smooth}};

// void delayedOff()
// {
//     delay(222);
//     off();
//     delay(11);
// }

void showInitializingLigths()
{
    selected_color.S = 1;
    selected_color.L = 0;
    for (int i = 0; i < 100; i++)
    {
        selected_color.L += 0.005;
        refreshLeds();
        delay(i < 50 ? 10 : 5);
    }
    for (int i = 0; i < 100; i++)
    {
        selected_color.L -= 0.005;
        refreshLeds();
        delay(i < 50 ? 5 : i > 90 ? 20
                                  : 10);
    }
}

void setup()
{
    Serial.begin(9600);
    pinMode(13, OUTPUT);
    pinMode(LedR, OUTPUT);
    pinMode(LedG, OUTPUT);
    pinMode(LedB, OUTPUT);
    pinMode(AUDIO_IN, INPUT);
    irrecv.enableIRIn();
    off();
    showInitializingLigths();
}

// determino cual programa correr dependiendo del código que llegó del control remoto.
void readRemoteControl()
{
    if (irrecv.decode(&results))
    {
        // para ver los códigos del remoto:
        dump();

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
    static unsigned long timer_alive = 0;
    long t = millis() - timer_alive;
    if (t < 111)
        digitalWrite(13, 1);
    else if (t < 222)
        digitalWrite(13, 0);
    else if (t < 333)
        digitalWrite(13, 1);
    else if (t < 444)
        digitalWrite(13, 0);
    else if (t > 3333)
        timer_alive = millis();
}

void loop()
{
    itsAlive();
    readRemoteControl();

    // el tono de color va rotando de a poquito...
    if (is_smooth_on)
    {
        selected_color.H += 0.001;
        if (selected_color.H > 360)
            selected_color.H = 0;
    }

    refreshLeds();
}
