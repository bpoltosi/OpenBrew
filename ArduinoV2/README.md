# Sistema de Controle e Monitoramento para Extração de Café (Versão 2.0 - V2 / Foco em Redes I2C)

## Resumo do Projeto Atual
A Versão 2.0 (V2) evolui o conceito original do assistente de café para uma arquitetura robusta voltada ao estudo e aplicação prática de **Redes de Comunicação (Protocolo I2C)**. Além de manter o monitoramento de massa da balança, a nova versão implementa um barramento serial síncrono mestre-escravo multi-nó. Agora, o sistema não apenas mede o peso do café, mas também sincroniza o tempo de extração exato através de um relógio de tempo real, demonstrando a capacidade de multiplexar múltiplos periféricos inteligentes (com endereços hexadecimais distintos) em um único par de fios (SDA e SCL).
Implementação de mais uma tela para exibir a contagem total do tempo da extração, ainda tenho que arrumar essa parte do código! mas ela servira mais para exemplificação do protocolo I2C - trabalho de redes de comunicação.

## Componentes Utilizados e Seus Usos

1. **Plataforma Arduino UNO (Mestre do Barramento I2C)**
   - **Uso:** Atua como o nó mestre (*Master*) da rede. É responsável por gerar o sinal de clock (`SCL`), coordenar o tráfego de dados na linha `SDA`, requisitar informações dos escravos, processar a lógica da balança e enviar os dados consolidados para exibição.

2. **Módulo de Relógio de Tempo Real - RTC DS1307 (Escravo I2C - Endereço `0x68`)**
   - **Uso:** Funciona como um nó escravo de **leitura** na rede I2C. Mantém a contagem precisa de segundos, minutos e horas (alimentado por bateria auxiliar), permitindo que o Arduino capture o horário exato ou cronometre a extração do café sob demanda.

3. **Display LCD 16x2 com Adaptador I2C (Escravo I2C - Endereço `0x27`)**
   - **Uso:** Funciona como um nó escravo de **escrita** na rede I2C. Recebe os pacotes de dados enviados pelo Arduino para desenhar na tela, de forma simultânea e sem conflitos com o RTC, as informações de tempo e peso.

4. **Célula de Carga + Módulo HX711 (Sensor de Massa)**
   - **Uso:** Mantido da versão anterior para a aquisição física do peso do café. Realiza a leitura piezelétrica da deformação mecânica e converte a massa em sinal digital enviado ao Arduino (ligado por portas digitais dedicadas, servindo de base analógica/digital para o sistema integrado).
