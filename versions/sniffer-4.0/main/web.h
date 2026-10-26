#ifndef WEB_H
#define WEB_H

#include <Arduino.h>

// ========== HTML ==========
const char HTML_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
<meta charset="utf-8">
<title>Modbus RTU Sniffer</title>
    <style>
        body{
            font-family:Arial,Helvetica,sans-serif;
            background:#f4f6f8;
            margin:0;
            color:#222
        }

        header{
            background:#0b3d5c;
            color:white;
            padding:16px 20px
        }

        header h1{
            font-size:22px;
            margin:0 0 10px 0
        }

        nav a{
            display:inline-block;
            color:white;
            text-decoration:none;
            background:#1565c0;
            padding:8px 12px;
            border-radius:8px;
            margin-right:8px
        }

        .wrap{
            max-width:1180px;
            margin:0 auto;
            padding:18px
        }

        .card{
            background:white;
            border-radius:12px;
            padding:14px;
            box-shadow:0 2px 8px rgba(0,0,0,.12)
        }

        .card h2{
            font-size:17px;
            margin:0 0 10px 0;
            color:#0b3d5c
        }

        input{
            box-sizing:border-box;
            padding:9px;
            border:1px solid #bbb;
            border-radius:8px;
            font-size:15px;
            background:white
        }

        button,.btn{
            display:inline-block;
            box-sizing:border-box;
            border:0;
            border-radius:8px;
            background:#1565c0;
            color:white;
            padding:10px 12px;
            font-size:15px;
            font-weight:bold;
            text-align:center;
            text-decoration:none;
            cursor:pointer
        }

        .row{
            display:flex;
            gap:8px;
            flex-wrap:wrap;
            align-items:center
        }

        .console{
            background:#111;
            color:#0f0;
            font-family:Consolas,monospace;
            font-size:13px;padding:10px;
            border-radius:8px
            ;height:360px;
            overflow-y:auto;
            white-space:pre-wrap;
            box-shadow:inset 0 0 5px rgba(0,0,0,.6)
        }

        .small{
            font-size:12px;
            color:#666;
            line-height:1.4
        }

        hr{
            border:0;
            border-top:1px solid #ddd;
            margin:16px 0
        }

    </style>
</head>
<body>
    <header>
        <h1>Modbus RTU Sniffer</h1>
        <nav>
            <a href="/">Painel</a>
            <a href="/config">Configuração</a>
        </nav>
    </header>

    <div class="wrap">
        <div class="card">
            <h2>Painel Principal</h2>

            <div class="row">
                <input id="fileName" type="text" value="sniffing_modbus.txt" />
                <button onclick="saveFile()">Salvar arquivo</button>
                <a class="btn" href="/config">Configurar Modbus</a>
            </div>

            <p class="small">Os dados recebidos pela UART são exibidos abaixo em tempo real via WebSocket</p>
            <hr />
            <h2>Console</h2>
            <div id="console" class="console">Sniffer Iniciado&#10;</div>       
        </div> 
    </div>
  <script>
    let ws = new WebSocket('ws://' + location.hostname + ':81/');

    ws.onopen = () => appendConsole("Conectado ao WebSocket");
    ws.onerror = (e) => appendConsole("Erro no WebSocket: ", e);
    ws.onclose = () => appendConsole("WebSocket desconectado");

    ws.onmessage = (e) => appendConsole(e.data);

    function appendConsole(text) {
        const consoleDiv = document.getElementById('console');
        consoleDiv.innerText += text + '\n';
        consoleDiv.scrollTop = consoleDiv.scrollHeight;
    }

    function saveFile() {
        let fileName = document.getElementById('fileName').value || "sniffing_modbus.txt";
        /* Código para salvar o arquivo no dispositivo do usuário sem utilizar o console */
        let content = document.getElementById('console').innerText;
        let blob = new Blob([content], { type: 'text/plain' });
        let link = document.createElement('a');
        link.href = URL.createObjectURL(blob);
        link.download = fileName;
        document.body.appendChild(link);
        link.click();
        document.body.removeChild(link);
    }

  </script>
</body>
)rawliteral";

const char CONFIG_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>

<head>
    <meta charset='UTF-8'>
    <meta name='viewport' content='width=device-width, initial-scale=1'>
    <title>Configuração Modbus</title>
    <style>
        body {
            font-family: Arial, Helvetica, sans-serif;
            background: #f4f6f8;
            margin: 0;
            color: #222;
        }

        header {
            background: #0b3d5c;
            color: white;
            padding: 16px 20px;
        }

        header h1 {
            font-size: 22px;
            margin: 0 0 10px 0;
        }

        nav a {
            display: inline-block;
            color: white;
            text-decoration: none;
            background: #1565c0;
            padding: 8px 12px;
            border-radius: 8px;
            margin-right: 8px;
        }

        .wrap {
            max-width: 1180px;
            margin: 0 auto;
            padding: 18px;
        }

        .status {
            background: #fff;
            border-left: 5px solid #1565c0;
            padding: 12px;
            border-radius: 10px;
            margin-bottom: 16px;
            box-shadow: 0 1px 5px rgba(0, 0, 0, .08);
        }

        .grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(280px, 1fr));
            gap: 14px;
        }

        .card {
            background: white;
            border-radius: 12px;
            padding: 14px;
            box-shadow: 0 2px 8px rgba(0, 0, 0, .12);
        }

        .card h2 {
            font-size: 17px;
            margin: 0 0 10px 0;
            color: #0b3d5c;
        }

        form {
            margin: 0 0 10px 0;
        }

        label {
            display: block;
            font-size: 13px;
            font-weight: bold;
            margin-top: 8px;
            margin-bottom: 4px;
        }

        input,
        select {
            width: 100%;
            box-sizing: border-box;
            padding: 9px;
            border: 1px solid #bbb;
            border-radius: 8px;
            font-size: 15px;
            background: white;
        }

        button,
        .btn {
            display: inline-block;
            width: 100%;
            box-sizing: border-box;
            border: 0;
            border-radius: 8px;
            background: #1565c0;
            color: white;
            padding: 10px 12px;
            font-size: 15px;
            font-weight: bold;
            text-align: center;
            text-decoration: none;
            cursor: pointer;
        }

        .danger button {
            background: #b3261e;
        }

        .toggle button {
            background: #5b2da0;
        }

        .display {
            background: #eef3f7;
            border-radius: 8px;
            padding: 10px;
            min-height: 42px;
            line-height: 1.45;
            margin-top: 10px;
            font-size: 14px;
            overflow-wrap: anywhere;
        }

        .ok {
            border-left: 5px solid #16833a;
        }

        .fail {
            border-left: 5px solid #b3261e;
        }

        .small {
            font-size: 12px;
            color: #666;
            line-height: 1.4;
        }

        .row {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 10px;
        }

        @media(max-width:700px) {
            .row {
                grid-template-columns: 1fr;
            }
        }
    </style>
</head>

<body>
    <header>
        <h1>Configuração Modbus</h1>
        <nav>
            <a href='/'>Painel</a>
            <a href='/config'>Configuração</a>
        </nav>
    </header>
    <div class='wrap'>
        <div class='card'>
            <h2>Parâmetros da Comunicação</h2>
            <form id="modbusForm">
                <div class="row">
                    <div>
                        <label>Slave ID</label>
                        <input type="number" id="id" min="1" max="247" value="1">
                    </div>
                    <div>
                        <label>Baud rate</label><select id="baud" name='baud'>
                            <option value='1200'>1200</option>
                            <option value='2400'>2400</option>
                            <option value='4800'>4800</option>
                            <option value='9600' selected>9600</option>
                            <option value='19200'>19200</option>
                            <option value='38400'>38400</option>
                            <option value='57600'>57600</option>
                            <option value='115200'>115200</option>
                        </select>
                    </div>
                    <div>
                        <label>Paridade</label>
                        <select id="parity" name='parity'>
                            <option value='N'>None</option>
                            <option value='E'>Even</option>
                            <option value='O'>Odd</option>
                        </select>
                    </div>
                    <div>
                        <label>Stop bits</label>
                        <select id="stopBits" name='stopBits'>
                            <option value='1'>1</option>
                            <option value='2'>2</option>
                        </select>
                    </div>
                </div>    
                <p class='small'>Data bits fixo em 8. O controle DE/RE  é fixo nesta versão.</p>
                <button type='submit'>Atualizar Serial/Modbus</button>                
            </form>
            <div id="status" class="display">Carregando configuração atual...</div>
        </div>
    </div>

    <script>
        async function loadConfig() {
            try {
                const r = await fetch('/api/modbus');
                const cfg = await r.json();

                document.getElementById('id').value = cfg.id;
                document.getElementById('baud').value = cfg.baud;
                document.getElementById('parity').value = cfg.parity;
                document.getElementById('stopBits').value = cfg.stopBits;

                showStatus('Configuração carregada: ID ' + cfg.id + ', ' + cfg.baud + 'bps, 8' + cfg.parity + cfg.stopBits, true);
            } catch (e) {
                showStatus('Falha ao carregar configuração atual.', false);
            }
        }

        document.getElementById('modbusForm').addEventListener('submit', async function(e) {
            e.preventDefault();

            const data = new URLSearchParams();
            data.append('id', document.getElementById('id').value);
            data.append('baud', document.getElementById('baud').value);
            data.append('parity', document.getElementById('parity').value);
            data.append('stopBits', document.getElementById('stopBits').value);

            try {
                const r = await fetch('/api/modbus', {
                    method: 'POST',
                    headers: {'Content-Type': 'application/x-www-form-urlencoded'},
                    body: data.toString()
                });

                const text = await r.text();

                if(r.ok) {
                    showStatus(text + '. A serial foi reiniciada com os novos parâmetros.', true);
                } else {
                    showStatus(text, false);
                }
            } catch (e) {
                showStatus('Erro ao enviar configuração ao ESP.', false);
            }
        });

        function showStatus(msg, ok) {
            const status = document.getElementById('status');
            status.innerText = msg;
            status.className = 'display ' + (ok ? 'ok' : 'fail');
        }

        loadConfig();
    </script>
</body>

</html>
)rawliteral";

#endif