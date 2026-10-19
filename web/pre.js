/*
 * pre.js
 *
 *  Created on: 19/10/2026
 *      Author: Victor
 *
 *  Codigo que o Emscripten injeta no inicio do index.js (--pre-js). Fica no
 *  mesmo escopo do runtime, entao pode usar FS/IDBFS diretamente - a pagina
 *  web/shell.html so chama os dois helpers abaixo em Module.
 *
 *  /save guarda o recorde (lokuras.rec) em um IDBFS (IndexedDB): o jogo
 *  grava como se fosse um arquivo normal e o navegador preserva o recorde
 *  entre visitas.
 */

Module.lokurasPrepararSave = function () {
  // Os caminhos do jogo ("img/fundo.png" etc.) sao relativos a raiz do FS.
  try { FS.chdir('/'); } catch (e) { console.warn('FS.chdir:', e); }

  // Estado visivel para o smoke test (tools/smoke-web.mjs): 'ok' quando o
  // /save montado no IndexedDB esta pronto; o recorde fica nesse diretorio.
  window.__lokuras_save = 'montando';

  try {
    FS.mkdir('/save');
    FS.mount(IDBFS, {}, '/save');

    // O jogo le o recorde ja em main(); o syncfs(true) e assincrono, entao
    // segura a inicializacao (run dependency) ate a leitura terminar.
    Module.addRunDependency('lokuras-save');
    FS.syncfs(true, function (err) {
      if (err) {
        window.__lokuras_save = 'erro: ' + err;
        console.warn('IDBFS (leitura do recorde):', err);
      } else {
        window.__lokuras_save = 'ok';
      }
      Module.removeRunDependency('lokuras-save');
    });
  } catch (e) {
    window.__lokuras_save = 'erro: ' + e;
    console.warn('IDBFS indisponivel:', e);
  }
};

Module.lokurasSincronizarSave = function () {
  try { FS.syncfs(false, function () {}); } catch (e) {}
};