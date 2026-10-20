/*
 * PlayStats.h
 *
 *  Created on: 17/10/2026
 *      Author: Victor
 *
 *  Estatisticas de partida e recorde persistido (lokuras.rec). Formato v2:
 *  linhas "chave=valor" com recorde por dificuldade, contadores acumulados
 *  (planetas/UFO destruidos em todas as partidas) e conquistas. O formato
 *  antigo (um inteiro = melhor pontuacao) ainda e lido e vira recorde da
 *  dificuldade NORMAL. Header-only: usa "magic statics" do C++11.
 */

#ifndef PLAYSTATS_H_
#define PLAYSTATS_H_

#include <cstdio>
#include <cstring>
#include <string>
#include "GameSettings.h"

// No build WebAssembly o recorde vai para /save (IDBFS, montado e sincronizado
// com o IndexedDB pela pagina web/shell.html); no desktop continua sendo um
// arquivo ao lado do jogo.
#ifdef __EMSCRIPTEN__
#define PLAYSTATS_FILE "/save/lokuras.rec"

// Definida em web/compat/sdl12_web.cpp: pede a gravacao imediata do /save no
// IndexedDB depois que o recorde muda no disco virtual.
extern "C" void lokurasWebSincronizarSave(void);
#else
#define PLAYSTATS_FILE "lokuras.rec"
#endif

// Conquistas persistidas (indices de achFlags()).
#define ACH_EARTH  0 // alcancar a Terra
#define ACH_HUNT   1 // destruir um UFO
#define ACH_50     2 // 50 planetas destruidos (somando todas as partidas)
#define ACH_NODMG  3 // alcancar a Terra sem levar dano
#define ACH_5MIN   4 // sobreviver 5 minutos em uma partida
#define ACH_COMBO  5 // emendar combo x3 numa partida
#define ACH_COUNT  6

class PlayStats
{
	public:
		// -1 = nenhum recorde ainda naquela dificuldade.
		static int& bestScore(int diff)
		{
			static int v[3] = { -1, -1, -1 };
			return v[diff >= 0 && diff < 3 ? diff : DIF_NORMAL];
		}

		static int& lastScore()     { static int v = 0; return v; }
		static int& lastTimeMs()    { static int v = 0; return v; }
		static int& lastDestroyed() { static int v = 0; return v; }
		static int& lastWave()      { static int v = 1; return v; }
		static int& lastCombo()     { static int v = 1; return v; }
		static bool& beatRecord()   { static bool v = false; return v; }

		static int& totalPlanets()  { static int v = 0; return v; }
		static int& totalUfos()     { static int v = 0; return v; }

		// Uma unica instancia do array (static de funcao inline e compartilhado
		// entre todas as traducoes).
		static bool* achFlags()
		{
			static bool flags[ACH_COUNT] = { false };
			return flags;
		}

		static bool ach(int i)
		{
			return (i >= 0 && i < ACH_COUNT) && achFlags()[i];
		}

		static const char* achName(int i)
		{
			switch(i)
			{
				case ACH_EARTH: return "ATERRISSAGEM";
				case ACH_HUNT:  return "CACADOR DE UFO";
				case ACH_50:    return "EXTERMINADOR (50 planetas)";
				case ACH_NODMG: return "INTACTO";
				case ACH_5MIN:  return "SOBREVIVENTE (5 MIN)";
				case ACH_COMBO: return "EM CHAMA (COMBO X3)";
				default:        return "";
			}
		}

		// Marca uma conquista; retorna true se ela acabou de ser desbloqueada
		// (grava o arquivo na hora para nao perder o progresso ao sair).
		static bool unlockAch(int i)
		{
			if(i < 0 || i >= ACH_COUNT)
				return false;

			if(achFlags()[i])
				return false;

			achFlags()[i] = true;
			writeFile();
			return true;
		}

		// Le o save (chamado ao abrir o menu). Migra o formato antigo.
		static void load()
		{
			FILE *f = fopen(PLAYSTATS_FILE, "r");
			if (!f)
				return;

			char line[64];
			bool sawKey = false;
			int legacy = -1;

			while (fgets(line, sizeof(line), f))
			{
				char *eq = strchr(line, '=');
				if (!eq)
				{
					int v = 0;
					if (sscanf(line, "%d", &v) == 1 && legacy < 0)
						legacy = v;
					continue;
				}

				*eq = 0;
				const char *key = line;
				int v = atoi(eq + 1);
				sawKey = true;

				if (!strcmp(key, "best0"))
					bestScore(DIF_FACIL) = v;
				else if (!strcmp(key, "best1"))
					bestScore(DIF_NORMAL) = v;
				else if (!strcmp(key, "best2"))
					bestScore(DIF_DIFICIL) = v;
				else if (!strcmp(key, "planets"))
					totalPlanets() = v;
				else if (!strcmp(key, "ufos"))
					totalUfos() = v;
				else if (!strcmp(key, "ach"))
				{
					for (int i = 0; i < ACH_COUNT; ++i)
					{
						// lista no formato "0,1,0,0,0,0"
						int bit = 0;
						if (sscanf(eq + 1 + i * 2, "%d", &bit) == 1)
							achFlags()[i] = (bit != 0);
					}
				}
			}
			fclose(f);

			// Formato antigo: um inteiro solto = recorde da dificuldade normal.
			if (!sawKey && legacy >= 0 && bestScore(DIF_NORMAL) < 0)
				bestScore(DIF_NORMAL) = legacy;
		}

		// Fecha uma partida: acumula contadores, atualiza o recorde da
		// dificuldade atual e persiste tudo. Define beatRecord().
		static void saveRun(int score, int destroyedThisRun, int ufosKilledThisRun)
		{
			totalPlanets() += destroyedThisRun;
			totalUfos()    += ufosKilledThisRun;

			if (totalPlanets() >= 50)
				achFlags()[ACH_50] = true;

			int diff = GameSettings::difficulty();
			beatRecord() = (bestScore(diff) < 0 || score > bestScore(diff));
			if (beatRecord())
				bestScore(diff) = score;

			writeFile();
		}

		// MM:SS (usado no HUD e na tela final).
		static std::string formatTime(int ms)
		{
			if (ms < 0)
				ms = 0;

			int s = ms / 1000;
			char buf[16];
			snprintf(buf, sizeof(buf), "%02d:%02d", s / 60, s % 60);
			return std::string(buf);
		}

	private:
		static void writeFile()
		{
			FILE *f = fopen(PLAYSTATS_FILE, "w");
			if (!f)
				return;

			fprintf(f, "best0=%d\n", bestScore(DIF_FACIL));
			fprintf(f, "best1=%d\n", bestScore(DIF_NORMAL));
			fprintf(f, "best2=%d\n", bestScore(DIF_DIFICIL));
			fprintf(f, "planets=%d\n", totalPlanets());
			fprintf(f, "ufos=%d\n", totalUfos());
			fprintf(f, "ach=");
			for (int i = 0; i < ACH_COUNT; ++i)
				fprintf(f, "%d%s", achFlags()[i] ? 1 : 0, i + 1 < ACH_COUNT ? "," : "\n");
			fclose(f);

			#ifdef __EMSCRIPTEN__
			lokurasWebSincronizarSave();
			#endif
		}
};

#endif /* PLAYSTATS_H_ */
