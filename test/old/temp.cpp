/**
 * VERSION 4
 * cambios desde la V3:
 * +cambie todos los colores a HSL...no va mas el RGB.
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
#include "IRremote.h"
#include "TimerEvent.h"

#define STEP_BRILLO 0.033
#define BRILLO_MAXIMO 0.5
#define TIME_SHIFT_COLOR 15
#define STEP_VELOCIDAD 10
#define VELOCIDAD_MINIMA 10
#define VELOCIDAD_NORMAL 100
#define VELOCIDAD_MAXIMA 500

#define RemoteCodesSize (int)(sizeof(RemoteCodes) / sizeof(CodeFunc))

// HSL: HUE [0-360], S [0-100%] L [0-100%]
#define HSL(h, s, l)            \
    {                           \
        hue = h / 360.0;        \
        saturation = s / 100.0; \
        brightness = l / 100.0; \
    }

//-- configuracion de los pines del arduino.
const int LedR = 9;
const int LedG = 5;
const int LedB = 6;
const int AUDIO_IN = A0;
const int RECV_PIN = 2;
const int DebugColorJumper = 3;
const int CambioHslHsv = 4;

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
unsigned int velocidad;
unsigned long time_shift_color = 0;

struct
{
    byte R, G, B;
} rgb;

// lo saqué de aca:
// https://codegolf.stackexchange.com/questions/150250/hsl-to-rgb-values
// The arguments are float hsl[3] and int rgb[3]
#define S(o, n) r[t[int(h[0]) / 60 * 3 + o] + o - 2] = (n + h[2] - c / 2) * 255;
void C(float *h, int *r)
{
    float g = 2 * h[2] - 1, c = (g < 0 ? 1 + g : 1 - g) * h[1], a = int(h[0]) % 120 / 60.f - 1;
    int t[] = {2, 2, 2, 3, 1, 2, 3, 3, 0, 4, 2, 0, 4, 1, 1, 2, 3, 1};
    S(0, c)
    S(1, c * (a < 0 ? 1 + a : 1 - a))
    S(2, 0)
}

//(esta función es para adaptarla al uso de HSV2RGB)
void HSL2RGB(float h, float s, float l)
{
    // h = hue; s = saturation; l = Lightness (luminancia)
    int RGB[3]; // resultado
    float H[3];
    H[0] = h * 360.0; // convierto los valores de hue [0-1] a [0-360]
    H[1] = s;
    H[2] = l;
    C(H, RGB);
    //uso la estructura RGB anterior para devolver los valores
    rgb.R = RGB[0];
    rgb.G = RGB[1];
    rgb.B = RGB[2];
}

void logColor()
{
    static byte R, G, B;
   // if (rgb.R - R + rgb.G - G + rgb.B - B == 0)
    {
        R = rgb.R;
        G = rgb.G;
        B = rgb.B;
        Serial.print("RGB(");
        Serial.print(rgb.R);
        Serial.print(',');
        Serial.print(rgb.G);
        Serial.print(',');
        Serial.print(rgb.B);
        Serial.print(") HSL(");
        Serial.print(hue);
        Serial.print(',');
        Serial.print(saturation);
        Serial.print(',');
        Serial.print(brightness);
        Serial.println(')');
    }
}

// enciende los leds segun el tono brillo etc.
void show_color()
{
    // convierte el espacio de color HSL a RGB:
    // el value es la luminancia: del negro (0%) al blanco (100%),
    // pasando por un 50% de tono puro.
    HSL2RGB(hue, saturation, brightness);

    analogWrite(LedR, rgb.R);
    analogWrite(LedG, rgb.G);
    analogWrite(LedB, rgb.B);
}

void shift_hue_color()
{
    if (millis() - time_shift_color > TIME_SHIFT_COLOR)
    {
        time_shift_color = millis();
        hue += 0.001;
        if (hue >= 1.0)
            hue = 0.0;
    }
}

float min = 1024, max = 0, atenuacion, brillo;
void tomarAudio()
{
    // leo el audio analogico
    int adc = analogRead(AUDIO_IN) / 1023.0;
    adc *= 2;
    if (adc < 0.01)
        adc = 0;
    brightness = adc;

    /*    TODO: ver como hacer..un control automatico de volumen 

    // leo el audio analogico
    int adc = analogRead(AUDIO_IN)/1023.0;

    // guardo los picos o extremos:
    if (adc < min)
        min = adc;
    if (adc > max)
        max = adc;

    // control automático de ganancia:
    atenuacion = (max / min) * 20.5;

    // le aplico un logaritmo para atenuar los picos máximos
    brillo = log(adc / atenuacion);

    //controlo saturacion: brillo va de 0 a 1
    if (brillo < 0)
        brillo = 0;
    if (brillo > 1.0)
        brillo = 1.0;

    // compuerta de sonido: lo apaga si no hay sonido.
    if (max - min <= 1)
        brillo = 0;

    // los extremos se van juntando o normalizando suavemente:
    min += 0.5;
    max -= 0.5;

    brightness = brillo / 10.0;*/
}

void refreshLeds()
{
    if (is_w_on)
    {
        //-- audiorritmico: enciende las luces según el valor de tensión que ingresa por A0:
        //-- subo solo el brillo, el tono lo da el HUE global.
        tomarAudio();
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
                //es un flash pero con el color seleccionado
                if (flash)
                    brightness = 0;
                else
                    brightness = 0.5;
            }
            else
            {
                //prende/apaga el flash.
                if (flash)
                {
                    HSL(0, 0, 100); // blanco
                }
                else
                {
                    HSL(0, 0, 0); // negro
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
                brightness += 0.01;
                if (brightness >= 0.5)
                {
                    sube = false;
                    brightness = 0.5;
                }
            }
            else
            {
                brightness -= 0.01;
                if (brightness <= 0)
                {
                    sube = true;
                    brightness = 0;
                }
            }
        }
    }
    show_color();
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
    HSL(0, 100, 25);
    Serial.println("ROJO");
}
void verde()
{
    HSL(120, 100, 50);
    Serial.println("VERDE");
}
void violeta()
{
    HSL(240, 100, 20);
    Serial.println("VIOLETA");
}
void rojo_claro()
{
    HSL(0, 100, 50);
    Serial.println("ROJITO");
}
void verde_claro()
{
    HSL(140, 100, 30);
    Serial.println("VERDITO");
}
void violetita()
{
    HSL(270, 100, 35);
    Serial.println("VIOLETITA");
}
void naranjon()
{
    HSL(24, 100, 50);
    Serial.println("NARANJON");
}
void celeste()
{
    HSL(195, 100, 60);
    Serial.println("CELESTE");
}
void magenta()
{
    HSL(340, 100, 30);
    Serial.println("MAGENTA");
}
void naranja()
{
    HSL(36, 100, 50);
    Serial.println("NARANJA");
}
void azulito()
{
    HSL(200, 100, 20);
    Serial.println("AZULITO");
}
void lila()
{
    HSL(300, 100, 35);
    Serial.println("LILA");
}
void amarillo()
{
    HSL(60, 100, 50);
    Serial.println("AMARILLO");
}
void azul()
{
    HSL(240, 100, 50);
    Serial.println("AZUL");
}
void rosa()
{
    HSL(315, 100, 60);
    Serial.println("ROSA");
}
void blanco()
{
    HSL(0, 0, 100);
    Serial.println("BLANCO");
}
void negro()
{
    HSL(0, 0, 0);
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
        if (brightness < BRILLO_MAXIMO)
            brightness += STEP_BRILLO;
        if (brightness > BRILLO_MAXIMO)
            brightness = BRILLO_MAXIMO;
        Serial.print("BRILLO ^ ");
        Serial.println(brightness);
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
        if (brightness > 0)
            brightness -= STEP_BRILLO;
        if (brightness < 0)
            brightness = 0;
        Serial.print("BRILLO v ");
        Serial.println(brightness);
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

void delayedOff()
{
    delay(222);
    off();
    delay(11);
}

void showInitializingLigths()
{
    hue = 0;
    saturation = 1;
    brightness = 0;
    for (int i = 0; i < 100; i++)
    {
        brightness += 0.005;
        show_color();
        delay(i < 50 ? 10 : 5);
    }
    for (int i = 0; i < 100; i++)
    {
        brightness -= 0.005;
        show_color();
        delay(i < 50 ? 5 : i > 90 ? 20 : 10);
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
    pinMode(DebugColorJumper, INPUT_PULLUP);
    pinMode(CambioHslHsv, INPUT_PULLUP);
    irrecv.enableIRIn();
    off();
    showInitializingLigths();
}

// determino cual programa correr dependiendo del código que llegó del control remoto.
void readRemoteControl()
{
    if (irrecv.decode(&results))
    {
        //para ver los códigos del remoto:
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
        shift_hue_color();

    refreshLeds();

    if (!digitalRead(DebugColorJumper))
        logColor();
}
