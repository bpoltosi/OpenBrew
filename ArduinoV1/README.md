# Monitor de Peso Inteligente para Café (Versão 1.0 - V1)

## Resumo do Projeto da Época
A Versão 1.0 (V1) nasceu como um conceito inicial para um sistema de auxílio ao preparo manual de café (métodos filtrados como V60 ou Prensa Francesa). O objetivo principal daquela época era criar um medidor de massa digital preciso e dedicado, que eliminasse o achismo na proporção entre o pó de café e a água. O sistema realizava a leitura do peso bruto através de uma célula de carga, tratava o sinal elétrico e exibia as informações em tempo real em um visor acoplado, garantindo repetibilidade e controle durante a extração.

## Componentes Utilizados e Seus Usos

1. **Plataforma Arduino UNO (Microcontrolador / "Cérebro")**
   - **Uso:** Responsável por executar o código-fonte, ler os sinais elétricos dos sensores, realizar o tratamento matemático (cálculo de calibração e médias) e enviar os comandos de exibição para a tela.

2. **Célula de Carga + Módulo Amplificador HX711 (Sensor de Peso)**
   - **Uso:** A célula de carga (barra de alumínio com extensômetros) deforma-se microscopicamente sob peso, alterando sua resistência elétrica. Como esse sinal é imperceptível, o módulo HX711 atua como um amplificador de alta precisão e conversor analógico-digital (ADC), traduzindo a variação física em dados numéricos compreensíveis para o Arduino.

3. **Display LCD 16x2 com Adaptador I2C (Interface de Saída)**
   - **Uso:** Tela de cristal líquido capaz de exibir 16 colunas por 2 linhas. A utilização do adaptador I2C (baseado no chip PCF8574) permitiu transformar a tela de um display de 16 pinos paralelos em um componente controlável por apenas 2 fios (SDA e SCL), servindo como o painel visual principal para o usuário acompanhar o peso atual.
