#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <HX711.h>

// Pinos do HX711
#define DOUT 3
#define SCK 2

HX711 scale;

// Endereço do LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Fator de calibração da sua célula de carga
float fator_calibracao = 420.5; 

void setup() {
  // Inicializa o LCD
  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("Monitoramento");
  lcd.setCursor(0, 1);
  lcd.print("de Peso");

  delay(2000);

  // Inicializa o HX711
  scale.begin(DOUT, SCK);
  
  // Aplica o fator de calibração ANTES de fazer a tara
  scale.set_scale(fator_calibracao);

  // Tara inicial (zera a balança)
  scale.tare();

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Sistema pronto");

  delay(1000);
  lcd.clear();
}

void loop() {

  if (scale.is_ready()) {

    // get_units(5) tira uma média de 5 leituras, deixando o peso mais estável
    float peso = scale.get_units(5);

    lcd.setCursor(0, 0);
    lcd.print("PESO ATUAL:     "); // Espaços extras limpam lixo na tela

    lcd.setCursor(0, 1);
    lcd.print(peso, 2); // O "2" indica que queremos 2 casa decimal
    lcd.print(" kg      "); // Unidade de medida em kg

  } else {

    lcd.setCursor(0, 0);
    lcd.print("HX711 ERRO      ");

  }

  delay(500);
}
