
/**********************************************************************
 *                       ARDUINO - SKYWRITTER 3D                       *
 * --------------------------------------------------------------------*
 * Autor:          Pedro Meléndez del Pozo                             *
 * Fecha:          05/06/2023                                          *
 * Descripción:    Control gestual de una cortina motorizada para      * 
 *                 aplicaciones domoticas                              *
 * Universidad:    UPM - Trabajo Fin de Grado                          *
 **********************************************************************/

 /* CONFIGURACION DE LOS PINES DEL ARDUINO NANO

  -------------SKYWRITTER 3D ---------------
 | 5V            VCC            Power       | 
 | A4            SDA            I2C Data    | 
 | A5            SCL            I2C Clock   | 
 | D2            RESET          Reset       |
 | D3            TRFR           Interrupt   |
 | GND           GND            Ground      | 
 |----------- CONTROL MOTORES --------------|
 | D6            OUTPUT         MOTOR_UP    | 
 | D5            OUTPUT         MOTOR_DOWN  |
 |----------- INTERRUPTORES ----------------|
 | D9            INPUT          SWITCH_UP   | 
 | D8            INPUT          SWITCH_DOWN |
  ------------------------------------------
*/

/////////////////////////////////////////////////////////////////////////////////////////////////
#include <Wire.h>
#include <skywriter.h>
/////////////////////////////////////////////////////////////////////////////////////////////////


//DEFINICION PINES DIGITALES
#define MOTOR_UP 6      //Control reles
#define MOTOR_DOWN 5

#define SWITCH_DOWN 8   //Deteccion pulsadores
#define SWITCH_UP 9

#define TRFR 3   // TRFR Pin of Skywriter
#define RESET 2  // Reset Pin of Skywriter

//Estado del motor
int status_UP;
int status_DOWN;

//Gestion de tiempos
unsigned long time;
unsigned long Tmax = 10000;//ms
long touch_time_off = 0;

/////////////////////////////////////////////////////////////////////////////////////////////////

//Definicion de funciones
void ScannerI2C();
void motor_up();       //Subir persiana
void motor_down();     //Bajar persiana
void stop_system();    //Parar motores

/////////////////////////////////////////////////////////////////////////////////////////////////
//                                    FUNCION DE SETUP                                         //
/////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {

  Serial.begin(115200);
  //Configuracion de pines
  pinMode(SWITCH_UP, INPUT);
  pinMode(SWITCH_DOWN, INPUT);
  pinMode(MOTOR_UP, OUTPUT);
  pinMode(MOTOR_DOWN, OUTPUT);

  //Estados de los motores
  status_UP = 0; 
  status_DOWN = 0;

  //Busqueda de dispositivos I2C conectados
  ScannerI2C();

  Skywriter.begin(TRFR, RESET); //Inicializacion de los parametros TRFR y RST del sensor

  Serial.println("-----------------------------------------------");
  Serial.println("        SkyWriter 3D - GESTURE CONTROL         ");
  Serial.println("-----------------------------------------------");

  //Funciones de habilitacion de gestos y toques
  //Skywriter.onTouch(touch); //Descomentar esta funcion para habilitar la deteccion de toques
  Skywriter.onGesture(gesture); //Deteccion de gestos
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//                                       BUCLE PRINCIPAL                                       //
/////////////////////////////////////////////////////////////////////////////////////////////////

void loop() 
{
  
  if (digitalRead(SWITCH_UP) == HIGH) 
  {

    motor_up();
    Serial.println("Pulsador subida ON");
    while (digitalRead(SWITCH_UP) == HIGH) 
    {
      
    }
    stop_system();
    Serial.println("Pulsador subida OFF");
  }

  if (digitalRead(SWITCH_DOWN) == HIGH) 
  {

    motor_down();
    Serial.println("Pulsador bajada ON");
    while (digitalRead(SWITCH_DOWN) == HIGH)
    {

    }
    stop_system();
    Serial.println("Pulsador bajada OFF");
  }
  
  Skywriter.poll();  // Poll for any updates from Skywriter
  
  if (touch_time_off > 0)
    touch_time_off--;

  if ((status_UP == 1 || status_DOWN == 1) && (millis()-time) > Tmax) //Desactivar motores tras un tiempo establecido
     stop_system();

}


///////////////////////////////////////////////////////////////////////////////////////////////////
//                        IMPLEMENTACION DE LA DETECCION DE GESTOS                               //
///////////////////////////////////////////////////////////////////////////////////////////////////
void gesture (unsigned char type_gesture) { 

  switch (type_gesture)
  {

    case SW_FLICK_SOUTH_NORTH : 
      Serial.println("SUBIEDO PERSIANA");
      motor_up();
      break;

    case SW_FLICK_NORTH_SOUTH:
      Serial.println("BAJANDO PERSIANA");
      motor_down();
      break;

    case SW_FLICK_WEST_EAST:
      Serial.println("PARAR MOTOR");
      stop_system();
      break;

    case SW_FLICK_EAST_WEST:
      Serial.println("PARAR MOTOR");
      stop_system();
      break;   

  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//            IMPLEMENTACION DE TOQUES TACTILES SOBRE LA SUPERFICIE DEL SENSOR                   //
///////////////////////////////////////////////////////////////////////////////////////////////////

/*void touch (unsigned char type_touch) 
{ 

  if (touch_time_off > 0) return;

  switch (type_touch) {

    case SW_TOUCH_NORTH:
      Serial.println("TOQUE ARRIBA");
      // Accion asociada al gesto //
      break; 

    case SW_TOUCH_SOUTH:
      Serial.println("TOQUE ABAJO");
      // Accion asociada al gesto //
      break;

    case SW_TOUCH_WEST:
      Serial.println("TOQUE IZQUIERDA");
      // Accion asociada al gesto //
      break;

    case SW_TOUCH_EAST:
      Serial.println("TOQUE DERECHA");
      flag_ID = RIGHT_Touch;
      // Accion asociada al gesto //
      break;
  }

  touch_time_off = 10000;
}*/

/////////////////////////////////////////////////////////////////////////////////////////////////
//                         FUNCIONES DE CONTROL DE LOS MOTORES                                 //
/////////////////////////////////////////////////////////////////////////////////////////////////

void motor_up()  //Subir persiana
{
  stop_system();
  delay(100);
  status_UP = 1;
  digitalWrite(MOTOR_UP, HIGH);
  time = millis();
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void motor_down()  //Bajar persiana
{
  stop_system();
  delay(100);
  status_DOWN = 1;
  digitalWrite(MOTOR_DOWN, HIGH);
  time = millis();
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void stop_system()  //Parar motores
{
  status_UP = 0;
  status_DOWN = 0;
  digitalWrite(MOTOR_UP, LOW);
  digitalWrite(MOTOR_DOWN, LOW);
}

void ScannerI2C()
{
    byte error, direccion;
    int nDispositivos;
    nDispositivos = 0;
    Serial.println("Buscando dispositivos... ");
 
    for(direccion = 8; direccion < 127; direccion++ )
    {
        Wire.beginTransmission(direccion);
        error = Wire.endTransmission();
 
        if (error == 0)
        {
            Serial.print("Dispositivo encontrado en la direccion: 0x");
            if (direccion < 16)
                Serial.print("0");
            Serial.print(direccion,HEX);
            Serial.println(" !");
            nDispositivos++;
        }
        else if (error == 4) 
        {
            Serial.print("Error en la direccion: 0x");
            if (direccion < 16)
                Serial.print("0");
            Serial.println(direccion,HEX);
        } 
    }
}

