# Sentinela Energético — v1 (limiar estatístico)

Nó embarcado (Arduino Uno) que lê um sinal físico, processa tudo localmente e detecta sozinho quando o comportamento sai do normal, acionando LED e buzzer. Não depende de PC nem de nuvem para decidir.

**Autor:** Wesley (Engenharia Elétrica, UEA)

## O que é e por que existe

O objetivo é monitorar o comportamento de um ambiente (no futuro, consumo de energia) e detectar anomalias na borda (*edge*). Como ainda não tenho sensor de corrente, usei um **NTC** (termistor) como sensor *proxy*: um elemento que consome mais energia esquenta, então temperatura é uma grandeza que se relaciona com consumo. O NTC **não mede corrente**; ele é um substituto para desenvolver e validar a lógica.

A arquitetura separa a camada do sensor (HAL) da lógica de decisão, para que o NTC possa ser trocado por um sensor de corrente sem reescrever a detecção.

Esta é a **v1**: detecção por limiar estatístico. A v2 vai adicionar um classificador leve treinado com dados reais e compará-lo com esta versão.

## Como funciona

A cada 200 ms:

```
sensor_hal_read()   -> lê o NTC (valor cru, 0 a 1023)
window_push()       -> guarda na janela deslizante (20 amostras)
detector_fixed()    -> compara com o limiar fixo
fsm_update()        -> atualiza o estado NORMAL/ANOMALIA
output_hal_update() -> LED e buzzer, só quando o estado muda
```

- **Janela deslizante:** buffer circular com as últimas 20 leituras, escrita em tempo constante.
- **Dois detectores implementados:** limiar **fixo** (valor cru acima de 590) e limiar **adaptativo** (valor acima de média + 3 desvios padrão da janela).
- **Máquina de estados com histerese:** só troca de estado após 3 leituras consecutivas concordando (cerca de 0,6 s), para ruído isolado não fazer o LED piscar.
- **A máquina de estados usa o limiar fixo**, pelo motivo abaixo.

## Decisões técnicas

**Simulação no PC antes do hardware.** Toda a lógica foi escrita e testada em C no PC, com um sensor simulado, e só depois portada para o Arduino trocando apenas a camada de sensor e de saída.

**Sinal simulado.** O sensor simulado gera um sinal que oscila perto de um ponto de repouso (passeio aleatório com reversão à média, taxa 0,07) e, raramente (0,2% por leitura), entra em uma anomalia sustentada de 30 a 80 leituras, com subida instantânea e recuperação gradual.

**Por que o detector adaptativo não comanda a máquina de estados.** Na simulação, o limiar adaptativo detecta o início de uma anomalia, mas deixa de sinalizar no meio dela: conforme a janela se enche de valores anômalos, a média e o desvio da própria janela se ajustam e a anomalia passa a parecer normal. O limiar fixo continuou sinalizando durante todo o evento. Por isso a v1 usa o fixo.

**Camadas desacopladas.** O resto do código só conhece a interface (`sensor_hal.h`, `output_hal.h`). No PC, as implementações são simuladas; no Arduino, usam `analogRead` e `digitalWrite`.

## Hardware

| Componente | Ligação |
|---|---|
| NTC | 5V, NTC, pino A0 |
| Resistor 10 kΩ | pino A0, GND |
| LED (com resistor em série) | pino D8 |
| Buzzer | pino D9 |

O NTC e o resistor formam um divisor de tensão: quando a temperatura sobe, a resistência do NTC cai e a leitura em A0 sobe.

## Estrutura

```
src/                         código da simulação no PC (hal, core, app, output)
platform/pc/main.c           ponto de entrada da simulação
platform/arduino/sentinela_arduino/
    sentinela_arduino.ino    ponto de entrada do Arduino
    src/                     cópia do núcleo + HAL e saída reais (.cpp)
Makefile                     compila a simulação no PC
```

## Como rodar

**Simulação no PC** (MSYS2 UCRT64 com `gcc` e `make`):

```
make
./sentinela.exe
```

**No Arduino:** abra `platform/arduino/sentinela_arduino/sentinela_arduino.ino` no Arduino IDE, selecione a placa Arduino Uno e faça o upload. No Serial Monitor (9600 baud), o sistema fica quieto. Digite `E` para ver o estado e `T` para ver a temperatura estimada; ela também aparece sozinha quando uma anomalia é detectada.

## Limitações

- **O sensor é um proxy.** O NTC mede temperatura, não corrente nem consumo.
- **O limiar fixo foi calibrado à mão**, com poucos testes de dedo no NTC. A leitura de repouso variou entre sessões (cerca de 485 a 550, com o ar-condicionado ligado ou desligado), então um limiar fixo é frágil quando o ambiente muda.
- **A temperatura em °C é uma estimativa**, que assume um NTC de 10 kΩ com beta 3950. A detecção usa a leitura crua, não os graus.
- **Código duplicado.** O núcleo existe em duas cópias (PC e Arduino), com caminhos de `#include` diferentes, e precisa ser mantido em sincronia.
- **Sem avaliação com métricas.** Não há dados rotulados nesta versão; a taxa de acertos e de falsos positivos dos detectores ainda não foi medida em dados reais.

## Próximos passos (v2)

1. Coletar dados reais rotulados em várias condições (dia e noite, com e sem ar-condicionado).
2. Treinar em Python uma árvore de decisão rasa e converter as regras para C.
3. Rodar o classificador no Arduino ao lado do limiar e comparar os dois nos mesmos dados.