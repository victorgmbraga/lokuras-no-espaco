/*
 * PlayStats.h
 *
 *  Created on: 17/10/2026
 *      Author: Victor
 *
 *  Estatisticas de partida e recorde persistido (lokuras.rec, um inteiro:
 *  pontuacao maxima). Header-only: as estaticas usam "magic statics" do
 *  C++11 e nao precisam de arquivo de implementacao.
 */

#ifndef PLAYSTATS_H_
#define PLAYSTATS_H_

#include <cstdio>
#include <string>
#define PLAYSTATS_FILE "lokuras.rec"

class PlayStats
{
	public:
		static int& bestScore()    { static int v = -1;  return v; } // -1 = nenhum recorde ainda
		static int& lastScore()    { static int v = 0;   return v; }
		static int& lastTimeMs()   { static int v = 0;   return v; }
		static int& lastDestroyed(){ static int v = 0;   return v; }
		static bool& beatRecord()  { static bool v = false; return v; }

		// Le o recorde do disco (chamado ao abrir o menu).
		static void load()
		{
			FILE *f = fopen(PLAYSTATS_FILE, "r");
			if (!f)
				return;

			int v = -1;
			if (fscanf(f, "%d", &v) == 1 && v >= 0)
				bestScore() = v;
			fclose(f);
		}

		// Salva o recorde se a partida o superou. Define beatRecord().
		static void save(int score)
		{
			beatRecord() = (bestScore() < 0 || score > bestScore());
			if (beatRecord())
				bestScore() = score;

			if (FILE *f = fopen(PLAYSTATS_FILE, "w"))
			{
				fprintf(f, "%d\n", bestScore());
				fclose(f);
			}
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
};

#endif /* PLAYSTATS_H_ */
