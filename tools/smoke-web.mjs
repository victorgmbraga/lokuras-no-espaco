#!/usr/bin/env node
/*
 * smoke-web.mjs
 *
 *  Created on: 19/10/2026
 *      Author: Victor
 *
 *  Teste de fumaca do build web: abre bin/web/index.html?selftest=1 em um
 *  navegador headless, espera o jogo desenhar quadros (o shell publica
 *  "LOKURAS_SELFTEST_PASS frames=N" no titulo) e, opcionalmente, salva um
 *  screenshot.
 *
 *  Uso:
 *    node tools/smoke-web.mjs [url]
 *
 *  Variaveis de ambiente:
 *    BROWSER / CHROME_PATH  caminho do navegador (padrao: Edge/Chrome comuns)
 *    SMOKE_TIMEOUT_MS       tempo maximo de espera (padrao: 60000)
 *    SMOKE_SCREENSHOT       caminho do PNG do screenshot (opcional)
 *
 *  Sai com codigo 0 se o jogo rodar e desenhar quadros; 1 caso contrario.
 */

import { spawn } from "node:child_process";
import { existsSync } from "node:fs";
import { mkdtemp, rm, writeFile } from "node:fs/promises";
import { tmpdir } from "node:os";
import { join } from "node:path";

const url = process.argv[2] || "http://127.0.0.1:8000/index.html?selftest=1";
const timeoutMs = Number(process.env.SMOKE_TIMEOUT_MS || 60000);
const screenshotPath = process.env.SMOKE_SCREENSHOT || "";

// --- Navegador --------------------------------------------------------------

function acharNavegador() {
  const candidatos = [
    process.env.BROWSER,
    process.env.CHROME_PATH,
    "C:\\Program Files (x86)\\Microsoft\\Edge\\Application\\msedge.exe",
    "C:\\Program Files\\Microsoft\\Edge\\Application\\msedge.exe",
    "C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe",
    "C:\\Program Files (x86)\\Google\\Chrome\\Application\\chrome.exe",
    "/usr/bin/google-chrome",
    "/usr/bin/google-chrome-stable",
    "/usr/bin/chromium",
    "/usr/bin/chromium-browser",
    "/Applications/Google Chrome.app/Contents/MacOS/Google Chrome",
  ];

  for (const caminho of candidatos) {
    if (caminho && existsSync(caminho)) return caminho;
  }
  return "";
}

const navegador = acharNavegador();
if (!navegador) {
  console.error("Navegador nao encontrado. Defina BROWSER ou CHROME_PATH.");
  process.exit(1);
}

// --- CDP (DevTools) ---------------------------------------------------------

class DevTools {
  constructor(ws) {
    this.ws = ws;
    this.proximoId = 1;
    this.pendentes = new Map();
    this.console = [];
    this.excecoes = [];

    ws.addEventListener("message", (evento) => {
      const mensagem = JSON.parse(String(evento.data));
      if (mensagem.id && this.pendentes.has(mensagem.id)) {
        const { resolver, rejeitar } = this.pendentes.get(mensagem.id);
        this.pendentes.delete(mensagem.id);
        if (mensagem.error) rejeitar(new Error(mensagem.error.message));
        else resolver(mensagem.result);
        return;
      }

      if (mensagem.method === "Runtime.consoleAPICalled") {
        const texto = (mensagem.params.args || [])
          .map((arg) => arg.value ?? arg.description ?? "")
          .join(" ");
        this.console.push({ tipo: mensagem.params.type, texto });
      } else if (mensagem.method === "Runtime.exceptionThrown") {
        this.excecoes.push(
          mensagem.params.exceptionDetails?.exception?.description ||
            mensagem.params.exceptionDetails?.text ||
            "excecao sem descricao",
        );
      }
    });
  }

  enviar(metodo, parametros = {}) {
    const id = this.proximoId++;
    return new Promise((resolver, rejeitar) => {
      this.pendentes.set(id, { resolver, rejeitar });
      this.ws.send(JSON.stringify({ id, method: metodo, params: parametros }));
    });
  }
}

function conectar(wsUrl) {
  return new Promise((resolver, rejeitar) => {
    const ws = new WebSocket(wsUrl);
    ws.addEventListener("open", () => resolver(new DevTools(ws)), { once: true });
    ws.addEventListener("error", () => rejeitar(new Error("falha no WebSocket do CDP")), { once: true });
  });
}

const dormir = (ms) => new Promise((resolver) => setTimeout(resolver, ms));

// Le a pagina usando CDP. Devolve { titulo, quadros, estado }.
async function lerPagina(devtools) {
  const resultado = await devtools.enviar("Runtime.evaluate", {
    expression: `JSON.stringify({
      titulo: document.title,
      quadros: window.__lokuras_frames | 0,
      estado: document.getElementById('estado')?.textContent || '',
      save: window.__lokuras_save || '',
      erro: window.__lokuras_erro || ''
    })`,
    returnByValue: true,
  });
  return JSON.parse(resultado.result.value);
}

// --- Execucao ---------------------------------------------------------------

const perfil = await mkdtemp(join(tmpdir(), "lokuras-smoke-"));
const argumentos = [
  "--headless=new",
  "--remote-debugging-port=0",
  `--user-data-dir=${perfil}`,
  "--no-sandbox",
  "--no-first-run",
  "--no-default-browser-check",
  "--disable-extensions",
  "--disable-background-networking",
  "--enable-unsafe-swiftshader",
  "--autoplay-policy=no-user-gesture-required",
  "--window-size=960,540",
  url,
];

console.log(`Navegador: ${navegador}`);
console.log(`Abrindo:   ${url}`);

const processo = spawn(navegador, argumentos, { stdio: ["ignore", "ignore", "pipe"] });

let saidaErro = "";
const portas = new Promise((resolver, rejeitar) => {
  // O Chrome do runner pode demorar para subir em maquinas carregadas.
  const prazo = setTimeout(() => rejeitar(new Error("o navegador nao abriu o CDP")), 45000);
  processo.stderr.on("data", (pedaco) => {
    saidaErro += pedaco;
    const encontrado = /DevTools listening on ws:\/\/127\.0\.0\.1:(\d+)\//.exec(saidaErro);
    if (encontrado) {
      clearTimeout(prazo);
      resolver(Number(encontrado[1]));
    }
  });
  processo.on("exit", (codigo) => {
    clearTimeout(prazo);
    rejeitar(new Error(`o navegador saiu antes do teste (codigo ${codigo})\n${saidaErro}`));
  });
});

let devtools = null;
let codigo = 1;

try {
  const porta = await portas;

  // Espera a aba da URL pedida aparecer na lista de alvos.
  let alvo = null;
  for (let tentativa = 0; tentativa < 40 && !alvo; tentativa++) {
    const alvos = await (await fetch(`http://127.0.0.1:${porta}/json/list`)).json();
    alvo = alvos.find((item) => item.type === "page" && item.url.startsWith("http"));
    if (!alvo) await dormir(250);
  }
  if (!alvo) throw new Error("a pagina do jogo nao apareceu no navegador");

  devtools = await conectar(alvo.webSocketDebuggerUrl);
  await devtools.enviar("Runtime.enable");
  await devtools.enviar("Page.enable");

  const inicio = Date.now();
  let ultimo = { titulo: "", quadros: 0, estado: "" };

  while (Date.now() - inicio < timeoutMs) {
    ultimo = await lerPagina(devtools);

    if (ultimo.titulo.startsWith("LOKURAS_SELFTEST_PASS")) {
      codigo = 0;
      break;
    }
    if (ultimo.titulo.startsWith("LOKURAS_SELFTEST_FAIL") || ultimo.erro) {
      break;
    }
    await dormir(250);
  }

  if (codigo === 0) {
    console.log(`OK: ${ultimo.titulo}`);
  } else {
    console.error(`FALHOU: ${ultimo.titulo || "(sem resultado)"} frames=${ultimo.quadros}`);
    console.error(`Estado da pagina: ${ultimo.estado}`);
  }

  // Alem do selftest: esconde a sobreposicao, aperta ENTER (comeca a partida)
  // e confirma que o jogo continua desenhando quadros - cobre teclado,
  // foco do canvas e a troca de estado menu -> jogo.
  if (codigo === 0) {
    // Retrato do menu antes de comecar (util para diagnostico visual).
    if (screenshotPath) {
      const menu = await devtools.enviar("Page.captureScreenshot", { format: "png" });
      const caminhoMenu = screenshotPath.replace(/\.png$/i, "-menu.png");
      await writeFile(caminhoMenu, Buffer.from(menu.data, "base64"));
      console.log(`Screenshot: ${caminhoMenu}`);
    }

    await devtools.enviar("Runtime.evaluate", {
      expression: "document.getElementById('comecar').click()",
    });
    const tecla = {
      key: "Enter",
      code: "Enter",
      windowsVirtualKeyCode: 13,
      nativeVirtualKeyCode: 13,
    };
    await devtools.enviar("Input.dispatchKeyEvent", { type: "keyDown", ...tecla });
    await devtools.enviar("Input.dispatchKeyEvent", { type: "keyUp", ...tecla });

    const antes = ultimo.quadros;

    // Segura A por ~1 s: alem de mover a nave, forca a rotacao do sprite, que
    // passa pelo rotozoomSurfaceXY da camada de compatibilidade.
    const girar = {
      key: "a",
      code: "KeyA",
      windowsVirtualKeyCode: 65,
      nativeVirtualKeyCode: 65,
      text: "a",
    };
    await devtools.enviar("Input.dispatchKeyEvent", { type: "rawKeyDown", ...girar });
    await dormir(1200);
    await devtools.enviar("Input.dispatchKeyEvent", {
      type: "keyUp",
      key: "a",
      code: "KeyA",
      windowsVirtualKeyCode: 65,
      nativeVirtualKeyCode: 65,
    });
    await dormir(800);

    // Dispara por ~0,4 s: cobre a criacao, atualizacao e render dos tiros.
    const atirar = {
      key: " ",
      code: "Space",
      windowsVirtualKeyCode: 32,
      nativeVirtualKeyCode: 32,
      text: " ",
    };
    await devtools.enviar("Input.dispatchKeyEvent", { type: "rawKeyDown", ...atirar });
    await dormir(400);
    await devtools.enviar("Input.dispatchKeyEvent", {
      type: "keyUp",
      key: " ",
      code: "Space",
      windowsVirtualKeyCode: 32,
      nativeVirtualKeyCode: 32,
    });
    await dormir(200);

    ultimo = await lerPagina(devtools);
    if (ultimo.quadros < antes + 30) {
      codigo = 1;
      console.error(
        `FALHOU: o jogo parou depois do ENTER/A/ESPACO (frames ${antes} -> ${ultimo.quadros})`,
      );
    } else {
      console.log(`OK: jogo rodando apos ENTER/A/ESPACO (frames ${antes} -> ${ultimo.quadros})`);
    }

    // O /save (IDBFS/IndexedDB) guarda o recorde: se a montagem falhou, o
    // navegador nao consegue persistir nada entre visitas.
    if (String(ultimo.save).startsWith("erro")) {
      codigo = 1;
      console.error(`FALHOU: /save indisponivel (${ultimo.save})`);
    } else {
      console.log(`OK: /save (IDBFS) ${ultimo.save || "montando"}`);
    }
  }

  const avisos = devtools.console.filter((item) => item.tipo === "warning" || item.tipo === "error");
  for (const aviso of avisos.slice(0, 20)) {
    console.error(`[console.${aviso.tipo}] ${aviso.texto}`);
  }
  for (const excecao of devtools.excecoes.slice(0, 10)) {
    console.error(`[excecao] ${excecao}`);
  }

  if (screenshotPath) {
    const captura = await devtools.enviar("Page.captureScreenshot", { format: "png" });
    await writeFile(screenshotPath, Buffer.from(captura.data, "base64"));
    console.log(`Screenshot: ${screenshotPath}`);
  }
} catch (erro) {
  console.error(`Erro no smoke test: ${erro.message}`);
} finally {
  if (devtools) {
    try {
      await devtools.enviar("Browser.close");
    } catch {
      // o navegador pode ja ter fechado
    }
  }
  if (!processo.killed) processo.kill();
  await rm(perfil, { recursive: true, force: true }).catch(() => {});
}

process.exit(codigo);
