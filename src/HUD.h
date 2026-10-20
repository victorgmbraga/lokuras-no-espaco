/*
 * HUD.h
 *
 *  Created on: 14/06/2013
 *      Author: Victor
 */

#ifndef HUD_H_
#define HUD_H_

#include "SDLBase.h"
#include "Text.h"
#include "PlayStats.h"

class HUD
{
	private:
		Text *lifeCounter;
		Text *commandsText;
		Text *commandsText3;
		Text *destroyedText;
		Text *timerText;
		Text *pauseText;
		Text *pauseHint;
		Text *bossLabel;
		Text *scoreText;    // pontos da partida (abaixo do cronometro)
		Text *comboText;    // "COMBO x1.5" enquanto a sequencia durar
		Text *fuelCounter;  // percentual de combustivel
		bool hintsOn;        // H mostra/esconde as dicas do rodape
		int hintsUntilMs;    // visiveis ate esse tempo de partida (renovavel com H)
		int life;
		int fuel;           // 0..100
		int bossLife, bossMax, bossWave;
		int pulseMs;        // tempo da partida (para as piscadas de alerta)
		float comboMult;
		int comboStreak;
		void displayLife();
		void displayFuel();
		void renderBossBar();
	public:
		HUD();
		virtual ~HUD();
		void update(int timeMs, int destroyed, int score, float comboMult, int comboStreak);
		void render();
		void renderPause();
		void setLife(int life);
		void setFuel(int fuel);
		void setBossLife(int life, int max, int wave = 1);
		void toggleHints(); // H: esconde ou reexibe as dicas por 15 s
};

#endif /* HUD_H_ */
