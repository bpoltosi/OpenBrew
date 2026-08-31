#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <HX711.h>

// =====================================================
// PINOS
// =====================================================

// HX711
#define HX711_DOUT 3
#define HX711_SCK  2

// Botões
#define BTN_ESQ 7
#define BTN_SEL 8
#define BTN_DIR 9

// I2C
#define LCD_ADDR 0x27
#define RTC_ADDR 0x68

// =====================================================
// OBJETOS
// =====================================================

LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);
HX711 balanca;

// =====================================================
// CONFIGURAÇÃO DA BALANÇA
// =====================================================

// Fator atual da sua balança
float fator_calibracao = 420.5;

// =====================================================
// RECEITAS
// =====================================================

struct Receita {
  byte pesoF1;   // gramas
  byte tempoF1;  // segundos

  byte pesoF2;   // gramas
  byte tempoF2;  // segundos
};

Receita receitas[3];

// Receita selecionada
byte receitaAtual = 0;

// =====================================================
// ESTADOS DO SISTEMA
// =====================================================
//
// 0 = Seleção da receita
// 1 = Preparação / Tara
// 2 = Extração Fase 1
// 3 = Extração Fase 2
// 4 = Conclusão
//
// =====================================================

byte estado = 0;

unsigned long tempoInicioFase = 0;

// =====================================================
// NVRAM DO DS1307
// =====================================================
//
// DS1307 possui RAM de usuário:
//
// 0x08 até 0x3F
//
// Vamos utilizar:
// 0x08 ~ 0x13 = receitas
// 0x14 = marcador de dados válidos
//
// =====================================================

#define NVRAM_BASE 0x08
#define MARCADOR 0x55

// =====================================================
// GRAVAR NA NVRAM
// =====================================================

void gravaNVRAM(byte posicao, byte dado) {

  Wire.beginTransmission(RTC_ADDR);

  Wire.write(NVRAM_BASE + posicao);
  Wire.write(dado);

  Wire.endTransmission();

  delay(5);
}

// =====================================================
// LER DA NVRAM
// =====================================================

byte leNVRAM(byte posicao) {

  Wire.beginTransmission(RTC_ADDR);

  Wire.write(NVRAM_BASE + posicao);

  Wire.endTransmission();

  Wire.requestFrom(RTC_ADDR, 1);

  if (Wire.available()) {
    return Wire.read();
  }

  return 0;
}

// =====================================================
// RECEITAS PADRÃO
// =====================================================

void carregarReceitasPadrao() {

  // ---------------------------------------------------
  // CAFÉ CURTO
  // ---------------------------------------------------

  receitas[0].pesoF1 = 50;
  receitas[0].tempoF1 = 30;

  receitas[0].pesoF2 = 150;
  receitas[0].tempoF2 = 90;

  // ---------------------------------------------------
  // CAFÉ MÉDIO
  // ---------------------------------------------------

  receitas[1].pesoF1 = 60;
  receitas[1].tempoF1 = 30;

  receitas[1].pesoF2 = 200;
  receitas[1].tempoF2 = 120;

  // ---------------------------------------------------
  // CAFÉ LONGO
  // ---------------------------------------------------

  receitas[2].pesoF1 = 70;
  receitas[2].tempoF1 = 40;

  receitas[2].pesoF2 = 250;
  receitas[2].tempoF2 = 150;
}

// =====================================================
// SALVAR RECEITAS
// =====================================================

void salvarReceitas() {

  for (byte i = 0; i < 3; i++) {

    gravaNVRAM(i * 4 + 0, receitas[i].pesoF1);
    gravaNVRAM(i * 4 + 1, receitas[i].tempoF1);
    gravaNVRAM(i * 4 + 2, receitas[i].pesoF2);
    gravaNVRAM(i * 4 + 3, receitas[i].tempoF2);
  }

  // Marca que existem dados válidos
  gravaNVRAM(12, MARCADOR);
}

// =====================================================
// CARREGAR RECEITAS DA NVRAM
// =====================================================

void carregarReceitas() {

  byte marcador = leNVRAM(12);

  // ---------------------------------------------------
  // Se não houver dados válidos
  // ---------------------------------------------------

  if (marcador != MARCADOR) {

    carregarReceitasPadrao();
    salvarReceitas();

  }

  // ---------------------------------------------------
  // Se já houver dados salvos
  // ---------------------------------------------------

  else {

    for (byte i = 0; i < 3; i++) {

      receitas[i].pesoF1 =
        leNVRAM(i * 4 + 0);

      receitas[i].tempoF1 =
        leNVRAM(i * 4 + 1);

      receitas[i].pesoF2 =
        leNVRAM(i * 4 + 2);

      receitas[i].tempoF2 =
        leNVRAM(i * 4 + 3);
    }
  }
}

// =====================================================
// LEITURA DOS BOTÕES
// =====================================================
//
// INPUT_PULLUP:
//
// HIGH = botão solto
// LOW  = botão pressionado
//
// Não utiliza while infinito.
//
// =====================================================

bool botaoPressionado(byte pino) {

  static bool estadoAnterior[10] = {
    HIGH, HIGH, HIGH, HIGH, HIGH,
    HIGH, HIGH, HIGH, HIGH, HIGH
  };

  static unsigned long ultimoDebounce[10] = {
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0
  };

  bool estadoAtual = digitalRead(pino);

  // Detecta transição:
  //
  // HIGH -> LOW
  //
  // Ou seja, botão acabou de ser pressionado.

  if (estadoAnterior[pino] == HIGH &&
      estadoAtual == LOW) {

    if (millis() - ultimoDebounce[pino] > 150) {

      ultimoDebounce[pino] = millis();

      estadoAnterior[pino] = estadoAtual;

      return true;
    }
  }

  estadoAnterior[pino] = estadoAtual;

  return false;
}

// =====================================================
// MOSTRAR MENU
// =====================================================

void mostrarMenu() {

  lcd.setCursor(0, 0);
  lcd.print("<- SELECIONA ->");

  lcd.setCursor(0, 1);

  if (receitaAtual == 0) {

    lcd.print("1: Cafe Curto   ");
  }

  else if (receitaAtual == 1) {

    lcd.print("2: Cafe Medio   ");
  }

  else {

    lcd.print("3: Cafe Longo   ");
  }
}

// =====================================================
// SETUP
// =====================================================

void setup() {

  Serial.begin(9600);

  // ===================================================
  // I2C
  // ===================================================

  Wire.begin();

  // ===================================================
  // LCD
  // ===================================================

  lcd.init();
  lcd.backlight();

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Iniciando...");

  lcd.setCursor(0, 1);
  lcd.print("Aguarde...");

  // ===================================================
  // BOTÕES
  // ===================================================

  pinMode(BTN_ESQ, INPUT_PULLUP);
  pinMode(BTN_SEL, INPUT_PULLUP);
  pinMode(BTN_DIR, INPUT_PULLUP);

  // ===================================================
  // HX711
  // ===================================================

  balanca.begin(HX711_DOUT, HX711_SCK);

  balanca.set_scale(fator_calibracao);

  // Tara inicial
  balanca.tare();

  // ===================================================
  // RECEITAS
  // ===================================================

  carregarReceitas();

  delay(1000);

  lcd.clear();
}

// =====================================================
// LOOP
// =====================================================

void loop() {

  // ===================================================
  // ESTADO 0
  // MENU DE SELEÇÃO
  // ===================================================

  if (estado == 0) {

    mostrarMenu();

    // -------------------------------------------------
    // BOTÃO ESQUERDO
    // -------------------------------------------------

    if (botaoPressionado(BTN_ESQ)) {

      if (receitaAtual > 0) {

        receitaAtual--;

        lcd.clear();
      }
    }

    // -------------------------------------------------
    // BOTÃO DIREITO
    // -------------------------------------------------

    if (botaoPressionado(BTN_DIR)) {

      if (receitaAtual < 2) {

        receitaAtual++;

        lcd.clear();
      }
    }

    // -------------------------------------------------
    // BOTÃO SELECIONAR
    // -------------------------------------------------

    if (botaoPressionado(BTN_SEL)) {

      estado = 1;

      lcd.clear();
    }
  }

  // ===================================================
  // ESTADO 1
  // PREPARAÇÃO / TARA
  // ===================================================

  else if (estado == 1) {

    lcd.setCursor(0, 0);
    lcd.print("Filtro vazio?   ");

    lcd.setCursor(0, 1);
    lcd.print("Pressione [SEL] ");

    // -------------------------------------------------
    // CONFIRMAR
    // -------------------------------------------------

    if (botaoPressionado(BTN_SEL)) {

      lcd.clear();

      lcd.setCursor(0, 0);
      lcd.print("Tarando...");

      // Faz nova tara
      balanca.tare();

      // Marca início da fase
      tempoInicioFase = millis();

      // Vai para fase 1
      estado = 2;

      lcd.clear();
    }
  }

  // ===================================================
  // ESTADOS 2 E 3
  // EXTRAÇÃO
  // ===================================================

  else if (estado == 2 || estado == 3) {

    // -------------------------------------------------
    // LEITURA DO HX711
    // -------------------------------------------------
    //
    // O HX711 está retornando a leitura em KG.
    //
    // Exemplo:
    //
    // 0.050 kg = 50 g
    // 0.150 kg = 150 g
    // 0.250 kg = 250 g
    //
    // Então convertemos KG -> G:
    //
    // pesoAtual = pesoKg * 1000
    //
    // -------------------------------------------------

    float pesoKg = balanca.get_units(5);

    float pesoAtual = pesoKg * 1000.0;

    // Evita valores negativos
    if (pesoAtual < 0) {
      pesoAtual = 0;
    }

    // -------------------------------------------------
    // TEMPO
    // -------------------------------------------------

    unsigned long tempoPassado =
      (millis() - tempoInicioFase) / 1000;

    // -------------------------------------------------
    // DADOS DA RECEITA
    // -------------------------------------------------

    byte pesoAlvo;
    byte tempoAlvo;

    if (estado == 2) {

      pesoAlvo = receitas[receitaAtual].pesoF1;
      tempoAlvo = receitas[receitaAtual].tempoF1;
    }

    else {

      pesoAlvo = receitas[receitaAtual].pesoF2;
      tempoAlvo = receitas[receitaAtual].tempoF2;
    }

    // -------------------------------------------------
    // SERIAL
    // -------------------------------------------------

    Serial.print("Peso: ");
    Serial.print(pesoAtual, 1);
    Serial.print(" g | Alvo: ");
    Serial.print(pesoAlvo);
    Serial.print(" g | Tempo: ");
    Serial.print(tempoPassado);
    Serial.print("/");
    Serial.println(tempoAlvo);

    // -------------------------------------------------
    // LCD - LINHA 1
    // -------------------------------------------------

    lcd.setCursor(0, 0);

    if (estado == 2) {
      lcd.print("F1 Tempo: ");
    }

    else {
      lcd.print("F2 Tempo: ");
    }

    lcd.print(tempoPassado);
    lcd.print("/");
    lcd.print(tempoAlvo);
    lcd.print("s ");

    // -------------------------------------------------
    // LCD - LINHA 2
    // -------------------------------------------------

    lcd.setCursor(0, 1);

    lcd.print(pesoAtual, 0);
    lcd.print("g / ");
    lcd.print(pesoAlvo);
    lcd.print("g   ");

    // -------------------------------------------------
    // FIM DA FASE
    // -------------------------------------------------

    if (tempoPassado >= tempoAlvo) {

      // ------------------------------------------------
      // FINAL DA FASE 1
      // ------------------------------------------------

      if (estado == 2) {

        estado = 3;

        tempoInicioFase = millis();

        lcd.clear();
      }

      // ------------------------------------------------
      // FINAL DA FASE 2
      // ------------------------------------------------

      else {

        estado = 4;

        lcd.clear();
      }
    }
  }

  // ===================================================
  // ESTADO 4
  // CONCLUSÃO
  // ===================================================

  else if (estado == 4) {

    lcd.setCursor(0, 0);
    lcd.print(" Cafe Pronto!   ");

    lcd.setCursor(0, 1);
    lcd.print("[SEL] para Menu ");

    // -------------------------------------------------
    // VOLTAR AO MENU
    // -------------------------------------------------

    if (botaoPressionado(BTN_SEL)) {

      estado = 0;

      receitaAtual = 0;

      lcd.clear();
    }
  }
}