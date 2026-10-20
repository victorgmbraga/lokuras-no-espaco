/*
 * HUD.cpp
 *
 *  Created on: 14/06/2013
 *      Author: Victor
 */

#include "HUD.h"
#include "GameSettings.h"

HUD::HUD()
{
	life = 100;
	fuel = 100;
	comboMult = 1.0f;
	comboStreak = 0;
	pulseMs = 0;
	hintsOn = true;
	hintsUntilMs = 12000;

	// Contador numerico ao lado da barra de vida.
	lifeCounter = new Text("ttf/PirataOne-Regular.ttf", "Vida: 100", 28, 0, {255, 255, 255}, 0, 0);
	lifeCounter->setPos(SCREEN_W / 80 + SCREEN_W * 3 / 10 + 14, 6);

	// Contador de combustivel (barra desenhada abaixo da de vida).
	fuelCounter = new Text("ttf/PirataOne-Regular.ttf", "Comb.: 100", 24, 0, {255, 200, 110}, 0, 0);
	fuelCounter->setPos(SCREEN_W / 80 + SCREEN_W * 3 / 10 + 14, 44);

	// Painel superior: planetas destruidos e cronometro.
	destroyedText = new Text("ttf/PirataOne-Regular.ttf", "Destruidos: 0", 28, 0, {255, 230, 130}, 0, 0);
	destroyedText->setPos(SCREEN_W / 2 - 300, 6);

	timerText = new Text("ttf/PirataOne-Regular.ttf", "Tempo: 00:00", 28, 0, {255, 230, 130}, 0, 0);
	timerText->setPos(SCREEN_W - 200, 6);

	// Pontuacao (abaixo do cronometro) e combo (abaixo dos destruidos).
	scoreText = new Text("ttf/PirataOne-Regular.ttf", "Pontos: 0", 28, 0, {130, 255, 160}, 0, 0);
	scoreText->setPos(SCREEN_W - 200, 36);

	comboText = new Text("ttf/PirataOne-Regular.ttf", "", 30, 0, {255, 120, 255}, 0, 0);

	// Barra de vida do UFO (chefe).
	bossLabel = new Text("ttf/PirataOne-Regular.ttf", "UFO", 24, 0, {255, 180, 120}, 0, 0);
	bossLife = 0;
	bossMax = 0;
	bossWave = 1;

	// Dicas de controle (rodape): somem sozinhas aos 12 s; H reexibe.
	commandsText = new Text("ttf/PirataOne-Regular.ttf", "W/S: impulso   A/D: gira   ESPACO/MOUSE1: atira", 24, 0, {255, 255, 255}, 0, 0);
	int textPosY = SCREEN_H - commandsText->getHeight();
	commandsText->setPos(14, textPosY);

	commandsText3 = new Text("ttf/PirataOne-Regular.ttf", "Paineis bloqueiam tiros: use-os de cobertura   |   H: dicas   |   F: menos flash   |   ESC: pausa", 24, 0, {255, 255, 255}, 0, 0);
	textPosY = SCREEN_H - commandsText->getHeight() * 2;
	commandsText3->setPos(14, textPosY);

	// Overlay de pausa.
	pauseText = new Text("ttf/PirataOne-Regular.ttf", "PAUSA", 72, 0, {255, 255, 255}, 0, 0);
	int posTextX = SCREEN_W / 2 - pauseText->getWidth() / 2;
	int posTextY = SCREEN_H / 2 - pauseText->getHeight() - 10;
	pauseText->setPos(posTextX, posTextY);

	pauseHint = new Text("ttf/PirataOne-Regular.ttf", "ESC: continuar   |   Q: menu   |   +/-: musica   |   9/0: sfx   |   M: mudo", 30, 0, {255, 255, 255}, 0, 0);
	posTextX = SCREEN_W / 2 - pauseHint->getWidth() / 2;
	posTextY = SCREEN_H / 2 + 24;
	pauseHint->setPos(posTextX, posTextY);
}

HUD::~HUD()
{
	delete(lifeCounter);
	delete(fuelCounter);
	delete(destroyedText);
	delete(timerText);
	delete(scoreText);
	delete(comboText);
	delete(commandsText);
	delete(commandsText3);
	delete(pauseText);
	delete(pauseHint);
	delete(bossLabel);
}

void HUD::update(int timeMs, int destroyed, int score, float comboMult, int comboStreak)
{
	pulseMs = timeMs;
	this->comboMult = comboMult;
	this->comboStreak = comboStreak;

	// Cronometro alinhado a direita (setText so re-renderiza quando muda).
	timerText->setText("Tempo: " + PlayStats::formatTime(timeMs));
	timerText->setPos(SCREEN_W - timerText->getWidth() - 16, 6);

	scoreText->setText("Pontos: " + std::to_string(score));
	scoreText->setPos(SCREEN_W - scoreText->getWidth() - 16, 36);

	destroyedText->setText("Destruidos: " + std::to_string(destroyed));
	destroyedText->setPos(SCREEN_W / 2 - destroyedText->getWidth() / 2 - 60, 6);

	// Combo visivel a partir de 2 abates emendados.
	if(comboStreak >= 2)
	{
		char buf[32];
		snprintf(buf, sizeof(buf), "COMBO x%.2g!", (double)comboMult);
		comboText->setText(buf);
		comboText->setPos(SCREEN_W / 2 - comboText->getWidth() / 2 - 60, 36);
	}
	else
		comboText->setText("");
}

void HUD::render()
{
	displayLife();
	displayFuel();
	lifeCounter->render();
	fuelCounter->render();
	destroyedText->render();
	timerText->render();
	scoreText->render();
	comboText->render();
	renderBossBar();
	// Dicas do rodape: visiveis no comeco da partida; H reexibe por 15 s.
	if(hintsOn && pulseMs <= hintsUntilMs)
	{
		commandsText->render();
		commandsText3->render();
	}
}

void HUD::toggleHints()
{
	// Se ja estao visiveis, esconde na hora; senao reexibe por 15 s.
	if(hintsOn && pulseMs <= hintsUntilMs)
		hintsOn = false;
	else
	{
		hintsOn = true;
		hintsUntilMs = pulseMs + 15000;
	}
}

void HUD::renderPause()
{
	SDLBase::drawOverlay(140);
	pauseText->render();
	pauseHint->render();
}

void HUD::displayLife()
{
	int maxWidth = SCREEN_W * 3 / 10;
	int barHeight = SCREEN_H / 30;
	int barWidth = maxWidth * life / 100;

	if(barWidth < 0)
		barWidth = 0;
	if(barWidth > maxWidth)
		barWidth = maxWidth;

	// Vida baixa: pisca em vermelho forte (ou vermelho fixo com flash
	// reduzido) para dar alerta.
	int color;
	if(life > 20)
		color = 0x00EA00;
	else if(GameSettings::reduceFlash())
		color = 0xC03030;
	else if((pulseMs / 200) % 2 == 0)
		color = 0xFF4040;
	else
		color = 0x601010;

	SDLBase::drawRect(SCREEN_W / 80, SCREEN_H / 60, barWidth, barHeight, color);
}

void HUD::displayFuel()
{
	int maxWidth = SCREEN_W * 3 / 10;
	int barHeight = SCREEN_H / 48;
	int barWidth = maxWidth * fuel / 100;

	if(barWidth < 0)
		barWidth = 0;
	if(barWidth > maxWidth)
		barWidth = maxWidth;

	// Acabando o combustivel: a barra pisca em vermelho (fixo com flash
	// reduzido).
	int color;
	if(fuel > 20)
		color = 0xFFB23A;
	else if(GameSettings::reduceFlash())
		color = 0xC03030;
	else if((pulseMs / 250) % 2 == 0)
		color = 0xFF4040;
	else
		color = 0x601010;

	int y = SCREEN_H / 60 + SCREEN_H / 30 + 8;
	SDLBase::drawRect(SCREEN_W / 80, y, barWidth, barHeight, color);
}

void HUD::setLife(int life)
{
	this->life = life;

	if(life < 0)
		life = 0;

	lifeCounter->setText("Vida: " + std::to_string(life));
}

void HUD::setFuel(int fuel)
{
	this->fuel = fuel;
	if(this->fuel < 0)
		this->fuel = 0;
	if(this->fuel > 100)
		this->fuel = 100;

	fuelCounter->setText("Comb.: " + std::to_string(this->fuel) + "%");
}

void HUD::setBossLife(int life, int max, int wave)
{
	if(life < 0)
		life = 0;
	if(max < 0)
		max = 0;

	if(life == bossLife && max == bossMax && wave == bossWave)
		return;

	bossLife = life;
	bossMax = max;
	bossWave = wave;

	if(max > 0 && life > 0)
	{
		if(wave > 1)
			bossLabel->setText("UFO ONDA " + std::to_string(wave) + " " + std::to_string(life) + "/" + std::to_string(max));
		else
			bossLabel->setText("UFO " + std::to_string(life) + "/" + std::to_string(max));
	}
}

void HUD::renderBossBar()
{
	if(bossMax <= 0 || bossLife <= 0)
		return;

	const int barW = 360;
	const int barH = 16;
	int x = SCREEN_W / 2 - barW / 2;
	// Abaixo das linhas de vida e combustivel para nao cobrir o rotulo "Comb.".
	int y = 80;

	// Fundo/borda, trilho e preenchimento proporcional a vida do chefe.
	SDLBase::drawRect(x - 3, y - 3, barW + 6, barH + 6, 0x000000);
	SDLBase::drawRect(x, y, barW, barH, 0x3A0A0A);

	int fill = barW * bossLife / bossMax;
	if(fill < 0)
		fill = 0;
	if(fill > barW)
		fill = barW;
	SDLBase::drawRect(x, y, fill, barH, 0xE23B3B);

	// Rotulo da onda centralizado abaixo da barra para nao brigar com o
	// painel do topo quando o nome fica longo ("UFO ONDA 2 30/30").
	bossLabel->setPos(x + barW / 2 - bossLabel->getWidth() / 2, y + barH + 6);
	bossLabel->render();
}
