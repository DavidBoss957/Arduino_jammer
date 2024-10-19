#include <ELECHOUSE_CC1101_SRC_DRV.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Configuración de la pantalla LCD con I2C (A5 - SCL, A4 - SDA)
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Cambia la dirección I2C si es necesario

// Pines para botones y LEDs
const int botonLectura = 4;  // Botón para lectura de frecuencia
const int botonAtaque = 7;   // Botón para iniciar ataque
const int ledLectura = 9;    // LED para modo de lectura
const int ledAtaque = 8;     // LED para modo de ataque

bool modoSeleccionado = false;  // Para saber si ya se seleccionó un modo

void setup() {
  // Inicialización de la pantalla LCD
  lcd.init();  // Cambiado a lcd.init() en lugar de lcd.begin()
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("1: Leer Frecuencia");
  lcd.setCursor(0, 1);
  lcd.print("2: Iniciar Ataque");

  // Configuración de pines para botones y LEDs
  pinMode(botonLectura, INPUT_PULLUP);
  pinMode(botonAtaque, INPUT_PULLUP);
  pinMode(ledLectura, OUTPUT);
  pinMode(ledAtaque, OUTPUT);
  
  // Inicialización de la comunicación serie
  Serial.begin(9600);
  
  // Inicialización del CC1101
  ELECHOUSE_cc1101.Init();
  Serial.println("CC1101 Inicializado correctamente");
}

void loop() {
  // Mostrar el menú hasta que se presione un botón
  if (!modoSeleccionado) {
    // Leer estado de los botones
    int estadoBotonLectura = digitalRead(botonLectura);
    int estadoBotonAtaque = digitalRead(botonAtaque);

    if (estadoBotonLectura == LOW) {  // Botón de lectura presionado
      delay(50);  // Debounce
      while (digitalRead(botonLectura) == LOW);  // Espera a que el botón se suelte
      modoSeleccionado = true;
      iniciarLecturaFrecuencia();  // Ejecuta la función de lectura de frecuencia
    }

    if (estadoBotonAtaque == LOW) {  // Botón de ataque presionado
      delay(50);  // Debounce
      while (digitalRead(botonAtaque) == LOW);  // Espera a que el botón se suelte
      modoSeleccionado = true;
      iniciarAtaque();  // Ejecuta la función de ataque
    }
  }
}

void iniciarLecturaFrecuencia() {
  // Encender LED de lectura
  digitalWrite(ledLectura, HIGH);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Leyendo Frecuencia");

  // Configurar el CC1101 en modo de recepción
  ELECHOUSE_cc1101.setMHZ(433.900);  // Ajusta la frecuencia para escuchar
  ELECHOUSE_cc1101.SetRx();
  Serial.println("Modo de lectura activado");

  // Bucle de lectura de frecuencia
  while (true) {
    if (digitalRead(botonLectura) == LOW) {  // Si el botón es presionado para salir
      delay(50);  // Debounce
      while (digitalRead(botonLectura) == LOW);  // Espera a que el botón se suelte
      break;  // Salir del modo de lectura
    }

    if (ELECHOUSE_cc1101.CheckRxFifo(100)) {  // Verifica si hay datos
      byte receivedData[64];  // Buffer de datos recibidos
      int length = ELECHOUSE_cc1101.ReceiveData(receivedData);

      // Mostrar la frecuencia leída en el monitor serie y LCD
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
    delay(100);  // Pausa para evitar saturación
  }

  // Salir del modo de lectura
  digitalWrite(ledLectura, LOW);  // Apagar LED al salir del modo
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("1:Leer Freq");
  lcd.setCursor(0, 1);
  lcd.print("2:Ataque");
  modoSeleccionado = false;  // Permitir volver al menú
}

void iniciarAtaque() {
  // Encender LED de ataque
  digitalWrite(ledAtaque, HIGH);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Iniciando Ataque");

  // Configurar el CC1101 para el ataque (transmisión continua)
  ELECHOUSE_cc1101.setMHZ(433.898);  // Frecuencia para el ataque
  ELECHOUSE_cc1101.SetTx();
  Serial.println("Modo de ataque activado");

  // Enviar paquetes de interferencia
  byte data[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
  while (true) {
    if (digitalRead(botonAtaque) == LOW) {  // Si el botón es presionado para salir
      delay(50);  // Debounce
      while (digitalRead(botonAtaque) == LOW);  // Espera a que el botón se suelte
      break;  // Salir del modo de ataque
    }

    ELECHOUSE_cc1101.SendData(data, sizeof(data));  // Enviar el paquete
    Serial.println("Paquete enviado");
    delay(500);  // Retardo entre envíos
  }

  // Salir del modo de ataque
  digitalWrite(ledAtaque, LOW);  // Apagar LED al salir del modo
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("1: Leer Frecuencia");
  lcd.setCursor(0, 1);
  lcd.print("2: Iniciar Ataque");
  modoSeleccionado = false;  // Permitir volver al menú
}