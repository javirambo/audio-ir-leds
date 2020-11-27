#include "IRremote.h"

#define LR 3
#define LG 5
#define LB 6
#define AUDIO A0
#define RECV_PIN 2
#define log(a, b)      \
  {                    \
    Serial.print(a);   \
    Serial.print(':'); \
    Serial.println(b); \
  }

IRrecv irrecv(RECV_PIN);
decode_results results;
int keyPressed = -1;
int r = 0, g = 70, b = 150;
boolean colorSet = false;

enum KEY_NAMES
{
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
} KEY;

// mapeo de códigos del IR y mi programa de luces:
unsigned int RemoteCodes55[] = {
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

// TinkercadRemoteCodes
unsigned int RemoteCodes[] = {
    0x00FF, OFF,
    0x807F, SMOOTH,
    0x40BF, FLASH,
    0x20DF, VIOLETA,
    0xA05F, FADE,
    0x609F, VIOLETITA,
    0x10EF, BRILLO_ABAJO,
    0x906F, VIOLETA,
    0x50AF, BRILLO_ARRIBA,
    0x30CF, ROJO_CLARO,
    0xB04F, VERDE_CLARO,
    0x708F, VIOLETITA,
    0x08F7, NARANJON,
    0x8877, CELESTE,
    0x48B7, MAGENTA,
    0x28D7, NARANJA,
    0xA857, AZULITO,
    0x6897, LILA,
    0x18E7, AMARILLO,
    0x9867, AZUL,
    0x58A7, ROSA};

#define RemoteCodesSize (sizeof(RemoteCodes) / sizeof(int))

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

void simpleCycle()
{
  if (++r > 255)
    r = 0;
  if (++g > 255)
    g = 0;
  if (++b > 255)
    b = 0;
  delay(3);
}

void refreshLeds()
{
  analogWrite(LR, r);
  analogWrite(LG, g);
  analogWrite(LB, b);
}

void flash()
{
  r = g = b = 0;
  refreshLeds();
  delay(100);
  r = g = b = 255;
  refreshLeds();
  delay(100);
}

void rgb(int red, int green, int blue)
{
  //si ya setie el color, no lo hago de nuevo:
  if (!colorSet)
  {
    colorSet = true;
    r = red;
    g = green;
    b = blue;
  }
}

// corre cíclicamante el programa seleccionado.
void runLightProgram()
{
  switch (keyPressed)
  {
  case BRILLO_ARRIBA:
    break;
  case BRILLO_ABAJO:
    break;
  case OFF:
    rgb(0, 0, 0);
    break;
  case ON:
    rgb(255, 255, 255);
    break;
  case ROJO:
    rgb(255, 0, 0);
    break;
  case VERDE:
    rgb(0, 255, 0);
    break;
  case VIOLETA:
    rgb(150, 22, 255);
    break;
  case W:
    break;
  case ROJO_CLARO:
    rgb(255, 58, 58);
    break;
  case VERDE_CLARO:
    rgb(38, 255, 139);
    break;
  case VIOLETITA:
    rgb(116, 37, 252);
    break;
  case FLASH:
    flash();
    break;
  case NARANJON:
    rgb(255, 110, 0);
    break;
  case CELESTE:
    rgb(0, 203, 255);
    break;
  case MAGENTA:
    rgb(255, 0, 255);
    break;
  case STROBE:
    break;
  case NARANJA:
    rgb(255, 153, 0);
    break;
  case AZULITO:
    rgb(0, 117, 175);
    break;
  case LILA:
    rgb(209, 117, 255);
    break;
  case FADE:
    break;
  case AMARILLO:
    rgb(255, 246, 0);
    break;
  case AZUL:
    rgb(0, 0, 255);
    break;
  case ROSA:
    rgb(255, 219, 232);
    break;
  case SMOOTH:
    rgb(0, 85, 170);
    simpleCycle();
    break;
  }
  log("keyPressed", keyPressed);
  refreshLeds();
}

// determino cual programa correr dependiendo del código que llegó del control remoto.
void readRemoteControl()
{
  if (irrecv.decode(&results))
  {

    //para ver los códigos del remoto:
    //dump();

    // busco si existe mapeada la tecla pulsada del IR:
    keyPressed = -1;
    //log("RemoteCodesSize", RemoteCodesSize);
    for (int i = 0; i < RemoteCodesSize; i++)
    {
      if ((results.value & 0xffff) == RemoteCodes[i])
      {
        //log("i", i);
        keyPressed = i;
        //algunos programas solo necesitan un color inicial.
        colorSet = false;
        break;
      }
    }
    //log("keyPressed", keyPressed);
    irrecv.resume(); // empezamos una nueva recepción
  }
}

void loop()
{
  readRemoteControl();
  runLightProgram();
}
