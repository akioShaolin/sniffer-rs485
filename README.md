# Projeto Sniffer

Sniffer de tráfego RS485/Modbus RTU para ESP8266. O projeto evolui da impressão de bytes em hexadecimal pela serial até um console no navegador, acessível pelo Wi-Fi do próprio dispositivo, com exportação da captura e configuração da comunicação.

O firmware mantém o transceptor em recepção e observa os bytes do barramento. A interpretação dos comandos e registradores é feita por quem analisa a captura: não há decodificação automática de Modbus nem verificação de CRC implementadas nos sketches.

## Organização

Os nomes antigos `Sniffer 1.0` a `Sniffer 4.0` correspondem, nesta organização, às pastas abaixo.

| Versão | Código principal | Características |
| --- | --- | --- |
| [1.0](versions/sniffer-1.0/main/main.ino) | `versions/sniffer-1.0/main/main.ino` | Captura bruta, RS485 a 9600 bps, debug por SoftwareSerial a 9600 bps, sem timestamp. |
| [2.0](versions/sniffer-2.0/main/main.ino) | `versions/sniffer-2.0/main/main.ino` | Debug a 115200 bps, timestamp e timeout de 4 ms. |
| [3.0](versions/sniffer-3.0/main/main.ino) | `versions/sniffer-3.0/main/main.ino` | Access Point Wi-Fi, servidor HTTP, WebSocket e download pelo navegador. |
| [4.0](versions/sniffer-4.0/main/main.ino) | `versions/sniffer-4.0/main/` | Gerenciamento da UART separado, painel e página de configuração Modbus. |

As imagens disponíveis estão em [screenshots/](screenshots/). Os arquivos `.txt` locais contêm dados reais de captura e anotações de análise; são ignorados pela regra `*.txt` do [.gitignore](.gitignore). Os exemplos desta documentação reproduzem apenas pequenos trechos desses arquivos.

## Evolução das versões

### Sniffer 1.0 — captura bruta

Usa `Serial` para receber o barramento a 9600 bps e `SoftwareSerial Debug(2, 14)` para imprimir os bytes como hexadecimal, precedidos de `[RX]`. Fecha a linha quando o silêncio observado ultrapassa 10 ms. A rotina também possui um `delay(10)`, portanto a separação é simples e depende do processamento do loop.

Não registra horário nem tempo relativo. A saída de debug precisa ser visualizada ou salva por um equipamento externo.

### Sniffer 2.0 — timestamp e debug mais rápido

Mantém a recepção a 9600 bps, aumenta o debug para 115200 bps e remove o atraso fixo da rotina de captura. Acrescenta um timestamp calculado por `millis() / 1000`, em segundos desde a inicialização, no início de cada quadro observado.

O timeout passa para 4 ms, buscando aproximar o silêncio de 3,5 caracteres do Modbus RTU nessa velocidade. A comparação no código é `> 4`, com resolução de milissegundos; não é uma medição exata do intervalo no fio. A visualização e o armazenamento continuam externos ao firmware.

### Sniffer 3.0 — captura pelo navegador

Mantém 9600 bps e timeout de 4 ms. Cria a rede Wi-Fi `Sniffer WiFi`, serve a interface na porta HTTP 80 e transmite os quadros por WebSocket na porta 81.

O HTML servido está embutido no próprio `main.ino`. O botão **Salvar** exporta o texto acumulado no console do navegador; o nome inicial é `sniffing_modbus.csv`, mas o conteúdo continua sendo texto de captura, sem uma estrutura de colunas CSV.

Os pinos `HALF_OR_FULL_PIN` e `BTN_PIN` aparecem nessa versão. O primeiro é mantido em nível baixo, selecionando half-duplex na montagem descrita nos comentários; o botão é configurado como entrada com pull-up, sem ação implementada no loop.

### Sniffer 4.0 — configuração pela interface

Divide responsabilidades entre:

- [main.ino](versions/sniffer-4.0/main/main.ino): captura, Wi-Fi, WebSocket e rotas HTTP.
- [ModbusManager.h](versions/sniffer-4.0/main/ModbusManager.h) e [ModbusManager.cpp](versions/sniffer-4.0/main/ModbusManager.cpp): parâmetros e reconfiguração da UART.
- [web.h](versions/sniffer-4.0/main/web.h): HTML do painel e da configuração efetivamente servidos pelo firmware.
- [platformio.ini](versions/sniffer-4.0/platformio.ini): ambiente `esp07`, plataforma `espressif8266`, framework Arduino e dependência `links2004/WebSockets@^2.7.3`.

A interface permite baud rates de 1200, 2400, 4800, 9600, 19200, 38400, 57600 e 115200 bps; paridade N/E/O; um ou dois stop bits; e slave ID de 1 a 247. Os data bits são fixos em 8. Os valores iniciais são ID 1 e 9600 bps, 8N1.

O slave ID é armazenado na configuração e devolvido pela API, mas não filtra os quadros recebidos nem transforma o sniffer em um escravo Modbus. As alterações reinicializam a UART e ficam somente em RAM: não há persistência após reiniciar o dispositivo.

O timeout é recalculado com a expressão inteira `((3500000UL * 11UL) / baud) / 1000UL`, com mínimo de 2 ms. Ela assume 11 bits por caractere para todas as configurações e trunca o resultado; em 9600 bps, o valor calculado é 4 ms. A captura compara o silêncio com esse valor usando `>`.

**Estado do projeto PlatformIO:** o firmware está em `main/`, enquanto `src/main.cpp` ainda contém apenas o exemplo inicial com `myFunction`. O `platformio.ini` não redefine o diretório de fontes. Assim, o build padrão não seleciona o firmware do sniffer. Esta documentação não altera essa estrutura nem afirma que o firmware foi compilado ou validado em hardware.

Os arquivos HTML em `src/` são arquivos separados; o código atual serve as constantes de `web.h`, sem carregá-los de um sistema de arquivos.

## Hardware e dependências

O ambiente PlatformIO identifica a placa como ESP-07, baseada em ESP8266. As versões 1.0 e 2.0 usam `Arduino.h` e `SoftwareSerial.h`; as versões 3.0 e 4.0 usam também `ESP8266WiFi.h`, `ESP8266WebServer.h` e `WebSocketsServer.h`.

| Sinal no código | GPIO / uso |
| --- | --- |
| `Serial` | UART de hardware para receber os bytes do transceptor RS485. |
| `DE_RE` | GPIO 12, mantido em LOW para recepção nas quatro versões. |
| `SoftwareSerial(2, 14)` | RX no GPIO 2 e TX no GPIO 14 para debug nas versões 1.0 e 2.0. |
| `HALF_OR_FULL_PIN` | GPIO 13 nas versões 3.0 e 4.0; LOW seleciona half-duplex. Os comentários citam um HC4066 selecionando entre dois transceptores. |
| `BTN_PIN` | GPIO 0 nas versões 3.0 e 4.0, entrada com pull-up. |

Esses números são GPIOs, não uma confirmação das marcações físicas de conectores. A pinagem completa da montagem, alimentação e modelo dos transceptores não estão estabelecidos pelos arquivos analisados. A ligação ao barramento depende de um transceptor RS485 compatível; os GPIOs não são uma interface elétrica RS485.

Para abrir as versões em formato Arduino, o sketch é `main/main.ino` dentro da pasta da versão, com o suporte ESP8266 e as bibliotecas correspondentes instalados. Na versão 4.0, os arquivos de `main/` devem permanecer juntos. A integração desse firmware ao build PlatformIO está pendente conforme descrito acima.

## Uso do console Wi-Fi (3.0 e 4.0)

Com o firmware correspondente gravado e a montagem conectada ao barramento:

1. Conecte o celular ou computador à rede **Sniffer WiFi**, senha **1234567890**, conforme as constantes do código.
2. Abra `http://192.168.4.1/`. A interface da versão 3.0 usa esse endereço fixo para o WebSocket; a versão 4.0 usa o hostname da página, na porta 81.
3. Na versão 4.0, abra **Configuração** (`/config`) e ajuste os parâmetros para a rede observada.
4. Mantenha o painel aberto para acompanhar os quadros recebidos.
5. Escolha o nome do arquivo e use **Salvar** (3.0) ou **Salvar arquivo** (4.0) para baixar o conteúdo do console no dispositivo do navegador.

Na versão 4.0, o nome inicial do download é `sniffing_modbus.txt`. O download contém também as mensagens presentes no console. Não há gravação de capturas na flash, cartão SD ou histórico recuperável no firmware: o texto se acumula na página aberta.

| Rota da versão 4.0 | Função |
| --- | --- |
| `GET /` | Painel de captura. |
| `GET /config` | Formulário de parâmetros. |
| `GET /api/modbus` | Configuração atual em JSON: `id`, `baud`, `parity`, `stopBits`. |
| `POST /api/modbus` | Recebe esses quatro campos como formulário e aplica os parâmetros. |

## Exemplos reais e interpretação

Os trechos abaixo foram lidos dos arquivos locais ignorados. Os nomes de equipamentos e grandezas são as anotações desses arquivos, não uma identificação automática ou uma garantia de compatibilidade. Os endereços são os valores transmitidos no protocolo; não se aplica aqui uma conversão para notação de registradores iniciada em 40001.

### Leitura de um registrador e resposta

Trecho de `versions/sniffer-3.0/REGS.txt`, anotado como **Model ID**:

```text
01 03 75 76 00 01 7F DC
01 03 02 01 BC B8 65
```

Na requisição, `01` é o endereço do escravo, `03` é a função de leitura de holding registers, `75 76` é o endereço inicial `0x7576` e `00 01` solicita um registrador. Os dois bytes finais ocupam o campo CRC.

Na resposta, `02` informa dois bytes de dados e `01 BC` corresponde ao valor bruto `0x01BC`, ou **444**, coerente com a anotação do arquivo. O sniffer exibe os bytes; a associação desse valor a um modelo exige o mapa do fabricante e o contexto do equipamento.

### Grandeza anotada sem escala confirmada

No mesmo arquivo, o trecho **PV1 Voltage** é:

```text
01 03 7D 10 00 01 9D A3
01 03 02 1E A2 30 5D
```

A leitura começa em `0x7D10` e retorna `0x1EA2`, ou **7842** como inteiro sem sinal. O nome da grandeza vem da anotação local. Não é possível afirmar o valor em volts sem confirmar o fator de escala e o tipo do registrador na documentação do equipamento.

### Comparação de consultas observadas

Pequenos trechos dos arquivos de apoio da versão 4.0:

| Origem local | Anotação | Bytes da consulta | Interpretação do pedido |
| --- | --- | --- | --- |
| `SNIFF DATALOGGER SOLAR VIEW 0..txt` | CHINT MONOFASICO | `01 03 1A 10 00 08 42 D1` | Escravo 1, função 03, oito registradores a partir de `0x1A10`. |
| `SNIFF DATALOGGER SOLAR VIEW 0..txt` | CHINT TRIFASICO DO | `01 04 00 06 00 04 11 C8` | Escravo 1, função 04 (input registers), quatro registradores a partir de `0x0006`. |
| `SNIFF ED100.txt` | MMW03-M22CH | `01 03 00 04 00 02 85 CA` | Escravo 1, função 03, dois registradores a partir de `0x0004`. |

Esses pedidos ajudam a comparar endereços, funções e quantidades consultadas por sistemas existentes. Uma consulta isolada não comprova a presença do modelo anotado, o significado dos registradores ou uma resposta válida. O campo CRC não é validado pelo firmware.

## Limites da captura

- `[RX]` identifica a recepção pela UART do sniffer. Pedidos e respostas observados no mesmo barramento recebem esse mesmo rótulo; não há classificação automática de direção.
- O timestamp das versões 2.0 a 4.0 representa o tempo relativo ao boot quando o primeiro byte é processado, não data e hora de calendário nem timestamp individual de cada byte.
- A separação de quadros depende do silêncio observado pelo loop, do escoamento do buffer serial e da resolução de `millis()`. Carga de debug ou Wi-Fi pode afetar a captura.
- Não há limite explícito para o tamanho do `String` de quadro nem para o histórico no console do navegador.
- A transmissão ao navegador depende de cliente WebSocket conectado; o firmware não mantém um histórico para clientes que se conectem depois.
- O estado de cliente é acompanhado por um único identificador (`wsClient`), embora o envio use broadcast; o comportamento com vários clientes não é gerenciado de forma completa.
- A API da versão 4.0 valida ID, paridade e stop bits, mas não valida o baud rate no backend. O formulário oferece valores predefinidos; entradas arbitrárias pela API não possuem a mesma restrição.

## Licença

Este projeto é disponibilizado sob a licença **MIT**, disponível em [LICENSE](LICENSE) e reproduzida abaixo. Copyright (c) 2025-2026 Pedro Akio Sakuma.

```text
MIT License

Copyright (c) 2025-2026 Pedro Akio Sakuma

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```
