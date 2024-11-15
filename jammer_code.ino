#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ELECHOUSE_CC1101_SRC_DRV.h>

// Configuración de la pantalla LCD con I2C (A5 - SCL, A4 - SDA)
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Cambia la dirección I2C si es necesario

// Pines para botones y LEDs
const int botonLectura = 4;
const int botonAtaque = 7;
const int botonMenu = 6;
const int ledLectura = 9;  // LED para modo de lectura
const int ledAtaque = 8;   // LED para modo de ataque

// Estados del sistema
enum Estado {
  MENU_PRINCIPAL,
  MODO_LECTURA,
  MODO_ATAQUE
};
Estado estadoActual = MENU_PRINCIPAL;

// Variables para controlar el envío en modo ataque
unsigned long ultimoEnvio = 0;  // Marca de tiempo del último envío
const unsigned long intervaloEnvio = 500;  // Intervalo de envío en milisegundos

void setup() {
  // Inicialización de la pantalla LCD
  lcd.init();
  lcd.backlight();
  mostrarMenuPrincipal();

  // Configuración de pines para botones y LEDs
  pinMode(botonLectura, INPUT_PULLUP);
  pinMode(botonAtaque, INPUT_PULLUP);
  pinMode(botonMenu, INPUT_PULLUP);
  pinMode(ledLectura, OUTPUT);
  pinMode(ledAtaque, OUTPUT);

  // Inicialización de la comunicación serie para depuración
  Serial.begin(9600);
  Serial.println("Sistema iniciado. Menú principal.");

  // Inicialización del CC1101
  ELECHOUSE_cc1101.Init();
  Serial.println("CC1101 Inicializado correctamente");
}

void loop() {
  switch (estadoActual) {
    case MENU_PRINCIPAL:
      gestionarMenu();
      break;
    case MODO_LECTURA:
      gestionarModoLectura();
      break;
    case MODO_ATAQUE:
      gestionarModoAtaque();
      break;
  }
}

void gestionarMenu() {
  // Comprobación de botón de lectura
  if (leerBoton(botonLectura)) {
    estadoActual = MODO_LECTURA;
    iniciarModoLectura();
  } 
  // Comprobación de botón de ataque
  else if (leerBoton(botonAtaque)) {
    estadoActual = MODO_ATAQUE;
    iniciarModoAtaque();
  }
}

// Función para iniciar el modo de lectura de frecuencia
void iniciarModoLectura() {
  digitalWrite(ledLectura, HIGH);  // Enciende LED de lectura
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Modo: Lectura");
  Serial.println("Modo de lectura activado");

  // Configurar el CC1101 en modo de recepción
  ELECHOUSE_cc1101.setMHZ(433.900);  // Ajusta la frecuencia para escuchar
  ELECHOUSE_cc1101.SetRx();
}

// Función para gestionar el modo de lectura
void gestionarModoLectura() {
  if (leerBoton(botonMenu)) {  // Salir del modo de lectura
    regresarMenu();
    digitalWrite(ledLectura, LOW);  // Apaga LED de lectura
    return;
  }

  if (ELECHOUSE_cc1101.CheckRxFifo(100)) {  // Verifica si hay datos
    byte receivedData[64];  // Buffer de datos recibidos
    int length = ELECHOUSE_cc1101.ReceiveData(receivedData);

    // Mostrar la frecuencia leída en el monitor serie
    Serial.print("Datos recibidos: ");
    for (int i = 0; i < length; i++) {
      Serial.print(receivedData[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
    
    // Mostrar en la pantalla LCD
    lcd.setCursor(0, 1);
    lcd.print("Frecuencia Leida");
  }
}

// Función para iniciar el modo de ataque
void iniciarModoAtaque() {
  digitalWrite(ledAtaque, HIGH);  // Enciende LED de ataque
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Modo: Ataque");
  Serial.println("Modo de ataque activado");

  // Configurar el CC1101 para el ataque (transmisión continua)
  ELECHOUSE_cc1101.setMHZ(433.898);  // Frecuencia para el ataque
  ELECHOUSE_cc1101.SetTx();
  
  // Reiniciar el temporizador de envío
  ultimoEnvio = millis();
}

// Función para gestionar el modo de ataque sin bloquear el sistema
void gestionarModoAtaque() {
  // Verificar si el botón de menú se ha presionado para salir del modo
  if (leerBoton(botonMenu)) {
    regresarMenu();
    digitalWrite(ledAtaque, LOW);  // Apaga el LED de ataque
    return;
  }

  // Enviar paquetes de interferencia a intervalos
  if (millis() - ultimoEnvio >= intervaloEnvio) {
    byte data[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
    ELECHOUSE_cc1101.SendData(data, sizeof(data));  // Enviar el paquete
    Serial.println("Paquete enviado en modo ataque");
    ultimoEnvio = millis();  // Actualizar el tiempo del último envío
  }
}

// Función para regresar al menú principal
void regresarMenu() {
  estadoActual = MENU_PRINCIPAL;
  mostrarMenuPrincipal();
}

// Función para mostrar el menú principal en la pantalla LCD
void mostrarMenuPrincipal() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("1: Leer Frecuencia");
  lcd.setCursor(0, 1);
  lcd.print("2: Iniciar Ataque");
}

// Función para leer botón con debounce
bool leerBoton(int pin) {
  static unsigned long ultimoTiempo = 0;
  static int ultimoEstado = HIGH;

  int estadoActual = digitalRead(pin);
  if (estadoActual == LOW && ultimoEstado == HIGH && (millis() - ultimoTiempo) > 50) {
    ultimoTiempo = millis();
    ultimoEstado = estadoActual;
    return true;
  }
  ultimoEstado = estadoActual;
  return false;
}
