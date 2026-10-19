/*
 * GameManager.cpp
 *
 *  Created on: 16/04/2013
 *      Author: Victor
 */

#include "StateGame.h"
#include <cmath>

void StateGame::load(int stack)
{
	// Recorde persistido (arquivo lokuras.rec).
	PlayStats::load();

	// Terra (escala assada na surface: hitbox == imagem renderizada)
	earthSprite = new Sprite("img/PlanetaTerra.png");
	earthSprite->scale(3);
	earth = new Earth(earthSprite, 1000, 1600, 100);

	// Lua (orbita a Terra e machuca a nave)
	moonSprite = new Sprite("img/Lua.png");
	moonSprite->scale(3);
	moon = new Moon(moonSprite, 1, earth);

	// Nave (sempre criada antes do UFO, que persegue ela)
	shipAnimation = new Animation("img/NaveSheet.png", 200, 4);
	ship = new AccObject(shipAnimation, 0, 0, 94, 100, 100);

	// UFO perseguidor autonomo
	ufoSprite = new Sprite("img/ufo.png");
	ufoSprite->scale(3);
	ufo = new FollowerObject(ufoSprite, 960, 435);
	ufo->setAutonomous(true);
	ufo->setChaseTarget(ship);
	ufo->setChaseSpeed(0.22);

	// Explosao (one-shot: toca uma vez e congela no ultimo quadro)
	boom = new Animation("img/BoomSheet.png", 200, 8);
	boom->setOneShot(true);
	boomX = 0;
	boomY = 0;
	boomActive = false;

	// Background
	bg = new Sprite("img/fundo.png");
	bg->resizeCover(SCREEN_W, SCREEN_H);

	// TileSet e TileMap
	tileSet = new TileSet(75, 75, "img/Tileset.png");
	tileMap = new TileMap("map/tileMap.txt", tileSet);

	// Planetas Vermelhos: surgem sozinhos. O primeiro nasce entre 2,5 e 4 s;
	// depois o intervalo cai conforme o tempo de partida aumenta.
	redPlanetSprite = new Sprite("img/PlanetaVermelho.png");
	redPlanetCooldown = 2500 + (rand() % 1500);

	// Power-ups (vida, escudo e turbo)
	powerUpSprites[0] = new Sprite("img/PowerVida.png");
	powerUpSprites[1] = new Sprite("img/PowerEscudo.png");
	powerUpSprites[2] = new Sprite("img/PowerTurbo.png");
	powerUpCooldown = 15000;

	// Som de coleta dos power-ups
	sfxPickup = new Audio("sfx/pickup.wav", AUDIOTYPE_SOUND);

	// Motor da nave: loop cujo volume acompanha a aceleracao.
	sfxEngine = new Audio("sfx/engine.wav", AUDIOTYPE_SOUND);
	sfxEngine->play(-1);
	sfxEngine->setVolume(0);

	// A camara singleton segue a nave.
	Camera::getInstance().bind(ship);

	// Música e sons
	sfx = new Audio("sfx/boom.wav", AUDIOTYPE_SOUND);
	music = new Audio("sfx/stateGame.ogg", AUDIOTYPE_MUSIC);
	music->play(-1);

	// HUD
	hud = new HUD();
	hud->setLife(ship->hp);

	// Cronometro da partida
	gameTimer = new Timer();
	gameTimer->start();
	gameTimeMs = 0;
	destroyedCount = 0;
	invulnTime = 0;
	gravX = 0;
	gravY = 0;
	shakeTime = 0;
	paused = false;

	// Variáveis de retorno
	returnState = 0;
	returnStack = 0;
}

int StateGame::unload()
{
	delete(bg);
	delete(redPlanetSprite);
	delete(earth);
	delete(earthSprite);
	delete(moon);
	delete(moonSprite);

	for(std::vector<Planet*>::iterator it = planetArray.begin(); it != planetArray.end(); ++it)
		delete(*it);
	planetArray.clear();

	for(std::vector<PowerUp*>::iterator it = powerUpArray.begin(); it != powerUpArray.end(); ++it)
		delete(*it);
	powerUpArray.clear();

	delete(powerUpSprites[0]);
	delete(powerUpSprites[1]);
	delete(powerUpSprites[2]);

	delete(ufo);
	delete(ufoSprite);
	delete(ship);          // NULL apos derrota: delete de NULL e seguro
	delete(shipAnimation);
	boomActive = false;
	delete(boom);
	delete(hud);
	delete(gameTimer);
	delete(sfxPickup);
	delete(sfxEngine);
	delete(music);
	delete(sfx);

	// A camera e singleton (vive fora deste estado); desvincula do objeto morto.
	Camera::getInstance().unBind();

	return returnStack;
}

void StateGame::checkCollision(int dt)
{
	// Power-ups: coleta (so ocorre com a nave viva).
	for(std::vector<PowerUp*>::iterator it = powerUpArray.begin(); it != powerUpArray.end(); )
	{
		if(ship && (*it)->collidesWith(ship))
		{
			switch((*it)->kind)
			{
				case PowerUp::VIDA:
					ship->hp = (ship->hp + 20 > 100) ? 100 : ship->hp + 20;
					break;
				case PowerUp::ESCUDO:
					ship->setShield(6000);
					break;
				case PowerUp::TURBO:
					ship->setBoost(BOOST_MULT, BOOST_TIME);
					break;
				default:
					break;
			}
			hud->setLife(ship->hp);
			sfxPickup->play(0);

			delete(*it);
			it = powerUpArray.erase(it);
		}
		else
			++it;
	}

	if(ship == NULL)
		return;

	// Planetas vermelhos: dano de 5, com i-frames (durante eles a nave
	// atravessa o planeta sem destrui-lo).
	std::vector<Planet*>::iterator it = planetArray.begin();
	while(it != planetArray.end())
	{
		if((*it)->collidesWith(ship))
		{
			if(invulnTime <= 0 && ship->getShieldTime() <= 0)
			{
				boomX = (*it)->x + (*it)->w/2;
				boomY = (*it)->y + (*it)->h/2;
				boomActive = true;
				boom->restart();

				ship->hp -= 5;
				hud->setLife(ship->hp);
				destroyedCount++;
				sfx->play(0);
				invulnTime = 800;
				shakeTime = 250;

				delete(*it);
				it = planetArray.erase(it);
			}
			else
				++it;
		}
		else
			++it;
	}

	// Lua: mesma regra dos planetas, mas a lua sobrevive (dano de 10).
	if(moon && moon->collidesWith(ship) && invulnTime <= 0 && ship->getShieldTime() <= 0)
	{
		boomX = moon->x + moon->w/2;
		boomY = moon->y + moon->h/2;
		boomActive = true;
		boom->restart();

		ship->hp -= 10;
		hud->setLife(ship->hp);
		sfx->play(0);
		invulnTime = 800;
		shakeTime = 250;
	}

	// UFO: ser capturada pelo perseguidor significa o fim (o escudo nao salvar,
	// apenas da uma ultima chance de escapar dele).
	if(ufo && ship->collidesWith(ufo) && ship->getShieldTime() <= 0)
		ship->hp = 0;

	if(ship->hp <= 0)
	{
		if(ship->hp < 0)
			ship->hp = 0;

		endGame(false);
		return;
	}

	// Vitoria: alcancar a Terra.
	if(earth && ship->collidesWith(earth))
	{
		endGame(true);
		return;
	}
}

void StateGame::addPlanet()
{
	if(ship == NULL)
		return;

	float sx = ship->x + ship->w/2;
	float sy = ship->y + ship->h/2;

	// Posicao aleatoria na tela, tentando nascer longe da nave.
	int planetX = Camera::getInstance().getX() + (rand() % SCREEN_W);
	int planetY = Camera::getInstance().getY() + (rand() % SCREEN_H);
	for(int tentativa = 0; tentativa < 20; ++tentativa)
	{
		planetX = Camera::getInstance().getX() + (rand() % SCREEN_W);
		planetY = Camera::getInstance().getY() + (rand() % SCREEN_H);

		float dx = planetX - sx;
		float dy = planetY - sy;
		if(sqrtf(dx*dx + dy*dy) > 260.0f)
			break;
	}

	PlanetRed *planetRed = new PlanetRed(redPlanetSprite, planetX, planetY, 20);
	planetArray.push_back(planetRed);
}

int StateGame::nextPlanetInterval()
{
	// O intervalo base cai 20 ms por segundo de partida: ~4,2 s no inicio,
	// ~3,0 s em 1 min, ~1,8 s em 2 min e no minimo 1,3 s (por volta de
	// 2 min 25 s). A variacao de +-20% evita um ritmo metronomico.
	int base = 4200 - 20 * (gameTimeMs / 1000);
	if(base < 1300)
		base = 1300;

	int variacao = base / 5;
	return base - variacao + (rand() % (2 * variacao + 1));
}

void StateGame::spawnPowerUp()
{
	// Maximo de 2 intens na tela ao mesmo tempo.
	if(powerUpArray.size() >= 2)
		return;

	// Proximo power-up nasce entre 15 e 25 segundos.
	powerUpCooldown = 15000 + (rand() % 10000);

	float sx = ship->x + ship->w/2;
	float sy = ship->y + ship->h/2;

	int kind = rand() % 3;
	int px = Camera::getInstance().getX() + (rand() % SCREEN_W);
	int py = Camera::getInstance().getY() + (rand() % SCREEN_H);
	for(int tentativa = 0; tentativa < 20; ++tentativa)
	{
		px = Camera::getInstance().getX() + (rand() % SCREEN_W);
		py = Camera::getInstance().getY() + (rand() % SCREEN_H);

		float dx = px - sx;
		float dy = py - sy;
		if(sqrtf(dx*dx + dy*dy) > 300.0f)
			break;
	}

	powerUpArray.push_back(new PowerUp(powerUpSprites[kind], px, py, kind));
}

void StateGame::endGame(bool win)
{
	// Estatisticas da partida (consumidas pela tela final).
	int score = destroyedCount * 50 + gameTimeMs / 1000;
	if(win)
		score += 500;

	PlayStats::lastTimeMs() = gameTimeMs;
	PlayStats::lastDestroyed() = destroyedCount;
	PlayStats::lastScore() = score;
	PlayStats::save(score);

	ship->stop();
	sfxEngine->setVolume(0);

	// "Solta" a camara (ficara livre na tela final).
	Camera::getInstance().unBind();

	if(win)
	{
		// Explosao centralizada no UFO (que foi destruido pela chegada).
		boomX = ufo->x + ufo->w/2;
		boomY = ufo->y + ufo->h/2;
		delete(ufo);
		ufo = NULL;
		returnStack = 1;
	}
	else
	{
		// Explosao centralizada na nave.
		boomX = ship->x + ship->w/2;
		boomY = ship->y + ship->h/2;
		delete(ship);
		ship = NULL;
		returnStack = 0;
	}

	boomActive = true;
	boom->restart();
	sfx->play(0);

	// Tela de vitoria/derrota.
	returnState = 3;
}

int StateGame::update(int dt)
{
	InputManager &im = InputManager::getInstance();

	// ESC pausa o jogo (voltar: ESC; menu: Q).
	if(im.isKeyDown(SDLK_ESCAPE))
	{
		paused = !paused;
		sfxEngine->setVolume(0);
	}

	if(paused)
	{
		if(im.quitGame())
			returnState = -1;
		else if(im.isKeyDown(SDLK_q))
			returnState = 1;

		return returnState;
	}

	// Fechar a janela encerra o jogo.
	if(im.quitGame())
		returnState = -1;

	// Tempo de partida (nao conta durante a pausa, pois retornamos acima).
	gameTimeMs += dt;

	// Planetas vermelhos (deriva lenta).
	for(std::vector<Planet*>::iterator it = planetArray.begin(); it != planetArray.end(); ++it)
		(*it)->update(dt);

	// Terra, Lua e UFO
	earth->update(dt);
	moon->update(dt);

	// UFO: rampa de dificuldade (perseguidor acelera com o tempo).
	if(ufo)
	{
		float chase = 0.22 + 0.14 * (gameTimeMs / 120000.0f);
		if(chase > 0.36)
			chase = 0.36;
		ufo->setChaseSpeed(chase);
		ufo->update(dt);
	}

	// Nave
	if(ship)
		ship->update(dt);

	// Gravidade da Terra: dentro do raio de influencia, a nave recebe uma
	// "corrente" que a puxa para o nucleo, com damping independente
	// do framerate para nao disparar sem controle.
	if(ship)
	{
		float ex = earth->x + earth->w/2;
		float ey = earth->y + earth->h/2;
		float sx = ship->x + ship->w/2;
		float sy = ship->y + ship->h/2;

		float dx = ex - sx;
		float dy = ey - sy;
		float d2 = dx*dx + dy*dy;

		if(d2 > 1.0f)
		{
			float d = sqrtf(d2);
			if(d < 600.0f && d > earth->w * 0.4f)
			{
				float a = 15.0f / d2; // px/ms^2
				if(a > 0.0005f)
					a = 0.0005f;

				gravX += a * dx / d * dt;
				gravY += a * dy / d * dt;
			}
		}

		float damp = powf(0.92f, dt / 16.7f);
		gravX *= damp;
		gravY *= damp;

		ship->x += gravX * dt;
		ship->y += gravY * dt;
	}

	// Rastro do motor: volume proporcional a aceleracao (0 quando parada).
	if(ship && ship->isAccelerating())
		sfxEngine->setVolume(90);
	else
		sfxEngine->setVolume(0);

	// Timers de efeito
	if(invulnTime > 0)
		invulnTime -= dt;
	if(shakeTime > 0)
		shakeTime -= dt;

	// Planetas vermelhos: surgem sozinhos. Quando a tela esta cheia, tenta de
	// novo em breve; senao agenda o proximo com o ritmo atual da partida.
	if(redPlanetCooldown > 0)
		redPlanetCooldown -= dt;

	if(redPlanetCooldown <= 0)
	{
		if(planetArray.size() < 12)
		{
			addPlanet();
			redPlanetCooldown = nextPlanetInterval();
		}
		else
			redPlanetCooldown = 500;
	}

	if(powerUpCooldown > 0)
		powerUpCooldown -= dt;
	else
		spawnPowerUp();

	// Explosao: uma unica passada por frame (one-shot).
	boom->update(dt);
	if(boomActive && boom->isFinished())
		boomActive = false;

	// Camara
	Camera::getInstance().update(dt);

	// Colisões
	checkCollision(dt);

	// HUD (estatisticas)
	hud->update(gameTimeMs, destroyedCount);

	return returnState;
}

void StateGame::renderObjective(float cameraX, float cameraY)
{
	// Posicao da Terra na tela.
	float ecx = earth->x + earth->w/2 - cameraX;
	float ecy = earth->y + earth->h/2 - cameraY;

	// Terra em tela: nada a indicar.
	if(ecx >= 0 && ecx <= SCREEN_W && ecy >= 0 && ecy <= SCREEN_H)
		return;

	float dx = ecx - SCREEN_W / 2.0f;
	float dy = ecy - SCREEN_H / 2.0f;
	float d = sqrtf(dx*dx + dy*dy);
	if(d < 1.0f)
		return;

	dx /= d;
	dy /= d;

	// Ponto na borda da tela (margem 70 px) na direcao da Terra.
	float t = 1e9f;
	if(dx > 0.0001f)
	{
		float cand = (SCREEN_W - 70 - SCREEN_W/2.0f) / dx;
		if(cand < t) t = cand;
	}
	else if(dx < -0.0001f)
	{
		float cand = (70 - SCREEN_W/2.0f) / dx;
		if(cand < t) t = cand;
	}
	if(dy > 0.0001f)
	{
		float cand = (SCREEN_H - 70 - SCREEN_H/2.0f) / dy;
		if(cand < t) t = cand;
	}
	else if(dy < -0.0001f)
	{
		float cand = (70 - SCREEN_H/2.0f) / dy;
		if(cand < t) t = cand;
	}

	float ax = SCREEN_W / 2.0f + dx * t;
	float ay = SCREEN_H / 2.0f + dy * t;

	// Chevron apontando para a Terra (SDL_gfx nao existe no Windows).
	float px = -dy;
	float py = dx;
	float hx = ax + dx * 30;
	float hy = ay + dy * 30;
	int cor = 0xEAD000;

	SDLBase::drawLine(ax - dx*12 + px*12, ay - dy*12 + py*12, hx, hy, cor, 1);
	SDLBase::drawLine(ax - dx*12 - px*12, ay - dy*12 - py*12, hx, hy, cor, 1);
	SDLBase::drawLine(ax - dx*12 + px*12, ay - dy*12 + py*12,
	                  ax - dx*12 - px*12, ay - dy*12 - py*12, cor, 1);
}

void StateGame::render()
{
	float cameraX = Camera::getInstance().getX();
	float cameraY = Camera::getInstance().getY();

	// Tremor de tela ao levar dano (efeito visual apenas: nao altera a
	// camara em si).
	if(shakeTime > 0)
	{
		int mag = 1 + shakeTime / 50;
		if(mag > 6)
			mag = 6;

		cameraX += (rand() % 2 == 0 ? 1 : -1) * mag;
		cameraY += (rand() % 2 == 0 ? 1 : -1) * mag;
	}

	// Background
	bg->render(0, 0);

	// Tilemap
	tileMap->render(cameraX, cameraY);

	// Planetas vermelhos
	for(std::vector<Planet*>::iterator it = planetArray.begin(); it != planetArray.end(); ++it)
		(*it)->render(cameraX, cameraY);

	// Terra
	earth->render(cameraX, cameraY);

	// Lua
	moon->render(cameraX, cameraY);

	// Power-ups
	for(std::vector<PowerUp*>::iterator it = powerUpArray.begin(); it != powerUpArray.end(); ++it)
		(*it)->render(cameraX, cameraY);

	// UFO (sem linhas de fila no modo perseguidor)
	if(ufo)
	{
		if(!ufo->isAutonomous())
			ufo->renderQueueLines(cameraX, cameraY);
		ufo->render(cameraX, cameraY);
	}

	// Nave (pisca durante os i-frames; anel quando o escudo esta ativo)
	if(ship)
	{
		if(invulnTime <= 0 || (gameTimeMs / 90) % 2 == 0)
			ship->render(cameraX, cameraY);

		if(ship->getShieldTime() > 0)
		{
			float scx = ship->x + ship->w/2 - cameraX;
			float scy = ship->y + ship->h/2 - cameraY;
			SDLBase::drawCircle((int)scx, (int)scy, 64, 0x00E5C8);
		}
	}

	// Explosao (uma vez, centrada no ponto da colisao)
	if(boomActive)
		boom->render(boomX - cameraX - boom->getClip()->w/2,
		             boomY - cameraY - boom->getClip()->h/2);

	// Seta de objetivo (Terra fora da tela)
	renderObjective(cameraX, cameraY);

	// HUD
	hud->render();

	// Sombra da pausa
	if(paused)
		hud->renderPause();
}
