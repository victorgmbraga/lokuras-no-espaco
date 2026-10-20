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
#include <ctime>
#include <map>
#include <string>
#include <vector>
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
#define ACH_EARTH  0 // alcancar a Terra (modo travessia)
#define ACH_HUNT   1 // destruir um UFO
#define ACH_50     2 // 50 planetas destruidos (somando todas as partidas)
#define ACH_NODMG  3 // vencer sem levar dano
#define ACH_5MIN   4 // sobreviver 5 minutos em uma partida
#define ACH_COMBO  5 // emendar combo x3 numa partida
#define ACH_SURV   6 // sobreviver 3 min no modo pontos
#define ACH_COUNT  7

class PlayStats
{
	public:
		// -1 = nenhum recorde ainda naquele modo e dificuldade.
		static int& bestScoreM(int mode, int diff)
		{
			static int v[2][3] = { {-1, -1, -1}, {-1, -1, -1} };
			if(mode < 0 || mode > 1)
				mode = MODE_TRAV;
			if(diff < 0 || diff > 2)
				diff = DIF_NORMAL;
			return v[mode][diff];
		}

		// Compat: recorde da travessia (formato antigo do save).
		static int& bestScore(int diff)
		{
			return bestScoreM(MODE_TRAV, diff);
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
				case ACH_SURV:  return "RESISTENCIA (3 MIN)";
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
				else if (!strncmp(key, "best", 4) && strlen(key) == 7 && key[5] == '_')
				{
					// Recorde por modo e dificuldade: "best<m>_<d>".
					int m = key[4] - '0';
					int d = key[6] - '0';
					if(m >= 0 && m <= 1 && d >= 0 && d <= 2)
						bestScoreM(m, d) = v;
				}
				else if (!strncmp(key, "daily:", 6) && strlen(key) == 14)
					dailyMap()[std::string(key + 6)] = v;
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

		// Fecha uma partida: acumula contadores, atualiza o recorde do
		// modo e dificuldade atuais e persiste tudo. Define beatRecord().
		// No desafio diario (?daily=1) tambem guarda a melhor do dia.
		static void saveRun(int score, int destroyedThisRun, int ufosKilledThisRun)
		{
			totalPlanets() += destroyedThisRun;
			totalUfos()    += ufosKilledThisRun;

			if (totalPlanets() >= 50)
				achFlags()[ACH_50] = true;

			int mode = GameSettings::gameMode();
			int diff = GameSettings::difficulty();
			beatRecord() = (bestScoreM(mode, diff) < 0 || score > bestScoreM(mode, diff));
			if (beatRecord())
				bestScoreM(mode, diff) = score;

			if(GameSettings::daily())
			{
				std::string hoje = todayKey();
				std::map<std::string, int>& dias = dailyMap();
				if(dias.find(hoje) == dias.end() || score > dias[hoje])
					dias[hoje] = score;
				while(dias.size() > 7)
					dias.erase(dias.begin()); // so a ultima semana
			}

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
		// Melhores do desafio diario: data AAAAMMDD -> pontos.
		static std::map<std::string, int>& dailyMap()
		{
			static std::map<std::string, int> v;
			return v;
		}

	public:
		// Dia local atual no formato AAAAMMDD (mesma base do --daily).
		static std::string todayKey()
		{
			time_t agora = time(NULL);
			struct tm *lt = localtime(&agora);
			char buf[16];
			snprintf(buf, sizeof(buf), "%04d%02d%02d",
				lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday);
			return std::string(buf);
		}

		// AAAAMMDD -> DD/MM para exibir no menu.
		static std::string dateLabel(const std::string& aaaammdd)
		{
			if(aaaammdd.size() != 8)
				return aaaammdd;
			return aaaammdd.substr(6, 2) + "/" + aaaammdd.substr(4, 2);
		}

		static int dailyBestToday()
		{
			std::map<std::string, int>& dias = dailyMap();
			std::map<std::string, int>::iterator it = dias.find(todayKey());
			return it == dias.end() ? -1 : it->second;
		}

		// Datas com recorde (ordenadas), para a linha do desafio no menu.
		static void recentDaily(std::vector<std::string>& datas, std::vector<int>& pontos)
		{
			datas.clear();
			pontos.clear();
			for(std::map<std::string, int>::iterator it = dailyMap().begin(); it != dailyMap().end(); ++it)
			{
				datas.push_back(it->first);
				pontos.push_back(it->second);
			}
		}

	private:
		static void writeFile()
		{
			FILE *f = fopen(PLAYSTATS_FILE, "w");
			if (!f)
				return;

			for(int m = 0; m < 2; ++m)
				for(int d = 0; d < 3; ++d)
					fprintf(f, "best%d_%d=%d\n", m, d, bestScoreM(m, d));
			fprintf(f, "planets=%d\n", totalPlanets());
			fprintf(f, "ufos=%d\n", totalUfos());
			fprintf(f, "ach=");
			for (int i = 0; i < ACH_COUNT; ++i)
				fprintf(f, "%d%s", achFlags()[i] ? 1 : 0, i + 1 < ACH_COUNT ? "," : "\n");
			for(std::map<std::string, int>::iterator it = dailyMap().begin(); it != dailyMap().end(); ++it)
				fprintf(f, "daily:%s=%d\n", it->first.c_str(), it->second);
			fclose(f);

			#ifdef __EMSCRIPTEN__
			lokurasWebSincronizarSave();
			#endif
		}
};

#endif /* PLAYSTATS_H_ */
