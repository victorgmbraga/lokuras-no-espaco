/*
 * GameSettings.h
 *
 *  Created on: 20/10/2026
 *      Author: Victor
 *
 *  Configuracao global da sessao: dificuldade escolhida no menu e seed da
 *  partida (linha de comando --seed N / --daily no desktop e no navegador).
 *  Header-only: acessores estaticos com "magic statics" do C++11.
 */

#ifndef GAMESETTINGS_H_
#define GAMESETTINGS_H_

#define DIF_FACIL   0
#define DIF_NORMAL  1
#define DIF_DIFICIL 2
#define DIF_QTD     3

class GameSettings
{
	public:
		// 0 = facil, 1 = normal, 2 = dificil (mudado com 1/2/3 no menu).
		static int& difficulty()   { static int v = DIF_NORMAL; return v; }

		// Seed informada por --seed (0 = nenhuma). --daily usa o dia atual.
		static unsigned int& seed()  { static unsigned int v = 0; return v; }
		static bool& daily()       { static bool v = false; return v; }

		static const char* difficultyName(int d)
		{
			if(d == DIF_FACIL)
				return "FACIL";
			if(d == DIF_DIFICIL)
				return "DIFICIL";
			return "NORMAL";
		}

		// Multiplicadores por dificuldade (index = difficulty()).
		static float planetSpawnMult()  { static const float v[3] = {1.3f, 1.0f, 0.72f}; return v[difficulty()]; }
		static float damageMult()       { static const float v[3] = {0.75f, 1.0f, 1.3f}; return v[difficulty()]; }
		static float chaseMult()        { static const float v[3] = {0.88f, 1.0f, 1.15f}; return v[difficulty()]; }
		static float enemyShotMult()    { static const float v[3] = {1.3f, 1.0f, 0.78f}; return v[difficulty()]; }
		static int ufoBaseHp()          { static const int v[3] = {14, 20, 28}; return v[difficulty()]; }
		static int ufoHpPerWave()       { static const int v[3] = {6, 10, 14}; return v[difficulty()]; }
		static int turretCount()        { static const int v[3] = {2, 4, 6}; return v[difficulty()]; }
		static float fuelDrainPerSec()  { static const float v[3] = {0.5f, 0.8f, 1.3f}; return v[difficulty()]; }
};

#endif /* GAMESETTINGS_H_ */
