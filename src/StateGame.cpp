/*
 * GameManager.cpp
 *
 *  Created on: 16/04/2013
 *      Author: Victor
 */

#include "StateGame.h"
#include <cmath>
#include <cstdio>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

// Dano base do contato com o UFO (30% da vida maxima da nave). O dano final
// e multiplicado pelo ajuste da dificuldade (GameSettings::damageMult()).
static const int UFO_COLLISION_DAMAGE = 30;
static const int MOON_COLLISION_DAMAGE = 10;
static const float COMBO_WINDOW_MS = 3000.0f;

// Utilitarios do gerador aleatorio do mapa/posicoes iniciais.
static float random01()
{
	return (float)rand() / (float)RAND_MAX;
}

static float randomRange(float minValue, float maxValue)
{
	if(maxValue <= minValue)
		return minValue;

	return minValue + (maxValue - minValue) * random01();
}

static float distanceBetween(float ax, float ay, float bx, float by)
{
	float dx = ax - bx;
	float dy = ay - by;
	return sqrtf(dx*dx + dy*dy);
}

static SDL_Color makeColor(unsigned char r, unsigned char g, unsigned char b)
{
	// SDL 1.2: o quarto componente e "unused" (alpha vem da propria surface).
	SDL_Color c;
	c.r = r; c.g = g; c.b = b;
	return c;
}

static int clampi(int v, int minV, int maxV)
{
	if(v < minV)
		return minV;
	if(v > maxV)
		return maxV;
	return v;
}

void StateGame::load(int stack)
{
	// Recorde persistido (arquivo lokuras.rec).
	PlayStats::load();

	// Background
	bg = new Sprite("img/fundo.png");
	bg->resizeCover(SCREEN_W, SCREEN_H);

	// TileSet e mapa aleatorio. O tamanho maximo e TILEMAP_MAX_SIZE x
	// TILEMAP_MAX_SIZE tiles; cada partida sorteia um mundo dentro do limite.
	// Os tiles da camada 1 sao solidos (quique/los de visada).
	tileSet = new TileSet(TILE_SIZE, TILE_SIZE, "img/Tileset.png");
	buildRandomMap();

	// Nave (criada antes da Terra e do UFO, que precisam da posicao dela)
	shipAnimation = new Animation("img/NaveSheet.png", 200, 4);
	ship = new AccObject(shipAnimation, 0, 0, 94, 100, 100);
	shipPrevX = ship->x;
	shipPrevY = ship->y;

	// Terra (escala assada na surface: hitbox == imagem renderizada).
	// No modo pontos nao ha Terra: sobreviver e a vitoria.
	bool pontos = (GameSettings::gameMode() == MODE_PTS);
	earthSprite = new Sprite("img/PlanetaTerra.png");
	earthSprite->scale(3);
	earth = pontos ? NULL : new Earth(earthSprite, 0, 0, 100);

	// UFO perseguidor autonomo (chefe da onda 1)
	ufoSprite = new Sprite("img/ufo.png");
	ufoSprite->scale(3);
	ufo = new FollowerObject(ufoSprite, 0, 0);
	// O PNG 32x32 escalado 3x tem margens transparentes em cima/embaixo;
	// usa uma hitbox central menor, sem deslocar o desenho do UFO.
	ufo->setHitbox(80, 48);
	ufo->setAutonomous(true);
	ufo->setChaseTarget(ship);
	ufo->setChaseSpeed(0.22);
	ufo->maxHp = GameSettings::ufoBaseHp();
	ufo->hp = ufo->maxHp;

	// Terra e UFO surgem longe da nave e um do outro.
	placeEarthAndUfo();

	// Lua (orbita a Terra ja posicionada e machuca a nave; fora no
	// modo pontos, junto com a Terra).
	moonSprite = new Sprite("img/Lua.png");
	moonSprite->scale(3);
	moon = (pontos || !earth) ? NULL : new Moon(moonSprite, 1, earth);

	// Torres: ancoradas nos paineis solidos, longe dos objetos principais.
	wave = 1;
	ufoRespawnTimer = 0;
	ufoShootTimer = (int)(2200 * GameSettings::enemyShotMult());
	ufoSummonTimer = 12000;
	ufosKilledThisRun = 0;
	chainTimer = 0;
	chainTick = 0;
	chainX = 0;
	chainY = 0;
	buildTurrets();

	// Planetas: a partida comeca com alguns ja espalhados pelo mapa; depois
	// surgem sozinhos com ritmo crescente. Vermelhos (2 tiros) e roxos
	// (4 tiros, mais lentos, dropam mais) se alternam nos spawns.
	redPlanetSprite = new Sprite("img/PlanetaVermelho.png");
	purplePlanetSprite = new Sprite("img/PlanetaRoxo.png");
	mineSprite = new Sprite("img/Mina.png");
	spawnInitialPlanets();
	redPlanetCooldown = (int)((2500 + (rand() % 1500)) * GameSettings::planetSpawnMult());

	// Explosao (one-shot: toca uma vez e congela no ultimo quadro)
	boom = new Animation("img/BoomSheet.png", 200, 8);
	boom->setOneShot(true);
	boomX = 0;
	boomY = 0;
	boomActive = false;

	// Power-ups (vida, escudo, turbo e combustivel)
	powerUpSprites[0] = new Sprite("img/PowerVida.png");
	powerUpSprites[1] = new Sprite("img/PowerEscudo.png");
	powerUpSprites[2] = new Sprite("img/PowerTurbo.png");
	powerUpSprites[3] = new Sprite("img/PowerCombustivel.png");
	powerUpCooldown = 15000;

	// Pool de textos flutuantes: 10 pequenos (placar/dano) + 4 grandes (avisos).
	for(int i = 0; i < 10; ++i)
	{
		Floater f;
		f.text = new Text("ttf/PirataOne-Regular.ttf", "", 22, 0, makeColor(255, 230, 130), 0, 0);
		f.active = false;
		f.big = false;
		f.x = f.y = 0;
		f.life = f.total = 0;
		floaters.push_back(f);
	}
	for(int i = 0; i < 4; ++i)
	{
		Floater f;
		f.text = new Text("ttf/PirataOne-Regular.ttf", "", 34, 0, makeColor(130, 255, 160), 0, 0);
		f.active = false;
		f.big = true;
		f.x = f.y = 0;
		f.life = f.total = 0;
		floaters.push_back(f);
	}

	// Pontuacao, combo e combustivel
	score = 0;
	comboStreak = 0;
	comboTimer = 0;
	comboMult = 1.0f;
	bestStreak = 0;
	survivalTimer = 0;
	fuel = 100.0f;
	fuelDamageTimer = 0;
	fuelAnnounced = false;
	fuelLowAnnounced = false;
	tookDamage = false;
	survWarnShown = false;
	tutThrustDone = tutTurnDone = tutShotDone = false;
	tutThrustShown = tutTurnShown = tutShotShown = tutEarthShown = false;
	enemyShotsFired = 0;
	debugMs = 0;

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
	hud->setBossLife(ufo->hp, ufo->maxHp, wave);
	hud->setFuel((int)fuel);

	// Cronometro da partida
	gameTimer = new Timer();
	gameTimer->start();
	gameTimeMs = 0;
	destroyedCount = 0;
	invulnTime = 0;
	gravX = 0;
	gravY = 0;
	shakeTime = 0;
	shootCooldown = 0;
	paused = false;

	// Protecao de saida: 3 s de escudo (com anel visivel) para a partida
	// nao acabar nos primeiros segundos, e dica do objetivo de cada modo.
	if(ship)
		ship->setShield(3000);
	if(GameSettings::gameMode() == MODE_PTS)
		toast("SOBREVIVA 3:00!");
	else
		toast("CHEGUE A TERRA!");

	// Variáveis de retorno
	returnState = 0;
	returnStack = 0;
}

void StateGame::buildRandomMap()
{
	// Dimensoes sorteadas dentro do limite (18 a 22 tiles de lado).
	int mapW = TILEMAP_MIN_SIZE + rand() % (TILEMAP_MAX_SIZE - TILEMAP_MIN_SIZE + 1);
	int mapH = TILEMAP_MIN_SIZE + rand() % (TILEMAP_MAX_SIZE - TILEMAP_MIN_SIZE + 1);

	worldWidth = mapW * TILE_SIZE;
	worldHeight = mapH * TILE_SIZE;

	tileMap = new TileMap(mapW, mapH, 2, tileSet);

	// A camera nao mostra fora do mundo (mundo maior que a tela agora).
	Camera::getInstance().setBounds(worldWidth, worldHeight);

	int totalTiles = tileSet->getTileCount();
	if(totalTiles <= 0)
		return;

	bool ocupado[TILEMAP_MAX_SIZE][TILEMAP_MAX_SIZE];
	for(int x = 0; x < TILEMAP_MAX_SIZE; ++x)
		for(int y = 0; y < TILEMAP_MAX_SIZE; ++y)
			ocupado[x][y] = false;

	// Ocupa de 50% a 70% das celulas: paineis espalhados, mas sem virar
	// uma parede solida sem espaco para o fundo estrelado aparecer.
	int alvo = (mapW * mapH) * (50 + rand() % 21) / 100;
	int colocados = 0;

	// Primeiro sorteia alguns blocos ("salas") de 2 a 4 tiles por lado.
	// Eles dao variedade visual; o restante e preenchido com tiles soltos.
	int salas = 8 + rand() % 7;
	for(int s = 0; s < salas && colocados < alvo; ++s)
	{
		int salaW = 2 + rand() % 3;
		int salaH = 2 + rand() % 3;
		if(salaW > mapW)
			salaW = mapW;
		if(salaH > mapH)
			salaH = mapH;

		int salaX = rand() % (mapW - salaW + 1);
		int salaY = rand() % (mapH - salaH + 1);
		int camada = (rand() % 100 < 60) ? 0 : 1;

		for(int y = salaY; y < salaY + salaH; ++y)
			for(int x = salaX; x < salaX + salaW; ++x)
			{
				if(ocupado[x][y])
					continue;

				tileMap->at(x, y, camada) = 1 + rand() % totalTiles;
				ocupado[x][y] = true;
				++colocados;
			}
	}

	// Completa as celulas restantes com tiles aleatorios isolados.
	int tentativas = 0;
	int limite = mapW * mapH * 10;
	while(colocados < alvo && tentativas < limite)
	{
		int x = rand() % mapW;
		int y = rand() % mapH;
		if(!ocupado[x][y])
		{
			int camada = (rand() % 100 < 60) ? 0 : 1;
			tileMap->at(x, y, camada) = 1 + rand() % totalTiles;
			ocupado[x][y] = true;
			++colocados;
		}
		++tentativas;
	}

	// Area de spawn da nave (canto 0,0): sempre livre, para a partida nao
	// comeca com a nave presa dentro de um painel solido.
	for(int x = 0; x <= 1; ++x)
		for(int y = 0; y <= 1; ++y)
		{
			if(tileMap->at(x, y, 0) != 0)
			{
				tileMap->at(x, y, 0) = 0;
				--colocados;
			}
			if(tileMap->at(x, y, 1) != 0)
			{
				tileMap->at(x, y, 1) = 0;
				--colocados;
			}
		}
}

void StateGame::placeEarthAndUfo()
{
	if(!ship || !ufo)
		return;

	float shipCX = ship->x + ship->w/2;
	float shipCY = ship->y + ship->h/2;

	// Distancias proporcionais ao tamanho do mundo: os dois objetos ficam
	// longe da nave e um do outro em qualquer um dos tamanhos sorteados.
	// Sem Terra (modo pontos), o UFO so precisa ficar longe da nave.
	float diagonal = sqrtf((float)(worldWidth * worldWidth + worldHeight * worldHeight));
	float minPlayerDist = diagonal * 0.50f;
	float minEarthUfoDist = diagonal * 0.42f;

	float earthCX = -1e9f;
	float earthCY = -1e9f;

	// Grade de posicoes validas (usada pela Terra e pelo UFO).
	struct Ponto { float x, y; };
	float bestDist = -1.0f;
	const float passo = 20.0f;

	if(earth)
	{
	// Terra: margem extra para a orbita da Lua (raio 200 + metade da Lua)
	// caber dentro do mapa.
	float earthHalfW = earth->w / 2;
	float earthHalfH = earth->h / 2;
	float earthMargin = earthHalfW + 250.0f;
	if(earthMargin > worldWidth / 2)
		earthMargin = worldWidth / 2;
	if(earthMargin > worldHeight / 2)
		earthMargin = worldHeight / 2;

	// Candidatas da Terra (longe da nave), sorteadas no final. A margem
	// tambem mantem a orbita da Lua dentro do mapa.
	std::vector<Ponto> candidatosTerra;

	float earthX = worldWidth / 2.0f;
	float earthY = worldHeight / 2.0f;
	bestDist = -1.0f;
	for(float cx = earthMargin; cx <= worldWidth - earthMargin; cx += passo)
		for(float cy = earthMargin; cy <= worldHeight - earthMargin; cy += passo)
		{
			float d = distanceBetween(cx, cy, shipCX, shipCY);

			if(d > bestDist)
			{
				bestDist = d;
				earthX = cx;
				earthY = cy;
			}

			if(d >= minPlayerDist)
			{
				Ponto p;
				p.x = cx;
				p.y = cy;
				candidatosTerra.push_back(p);
			}
		}

	if(!candidatosTerra.empty())
	{
		int escolhido = rand() % (int)candidatosTerra.size();
		earthX = candidatosTerra[escolhido].x;
		earthY = candidatosTerra[escolhido].y;
	}

	earth->x = earthX - earthHalfW;
	earth->y = earthY - earthHalfH;

	earthCX = earth->x + earthHalfW;
	earthCY = earth->y + earthHalfH;
	} // fim do posicionamento da Terra (modo travessia)

	// UFO: tambem longe da nave e, alem disso, longe da Terra.
	float ufoHalf = ufoSprite->getRenderWidth() / 2.0f;
	float ufoMargin = ufoHalf + 4.0f;
	if(ufoMargin > worldWidth / 2)
		ufoMargin = worldWidth / 2;
	if(ufoMargin > worldHeight / 2)
		ufoMargin = worldHeight / 2;

	// O UFO tem duas restricoes (nave e Terra). Em vez de tentar sorteios
	// independentes, varre uma grade de posicoes validas e escolhe uma ao
	// acaso: garante as distancias minimas sem perder a variedade.
	std::vector<Ponto> candidatos;

	float ufoCX = worldWidth / 2.0f;
	float ufoCY = worldHeight / 2.0f;
	bestDist = -1.0f;

	for(float cx = ufoMargin; cx <= worldWidth - ufoMargin; cx += passo)
		for(float cy = ufoMargin; cy <= worldHeight - ufoMargin; cy += passo)
		{
			float dPlayer = distanceBetween(cx, cy, shipCX, shipCY);
			float dEarth = distanceBetween(cx, cy, earthCX, earthCY);
			float scorePoint = dPlayer < dEarth ? dPlayer : dEarth;

			if(scorePoint > bestDist)
			{
				bestDist = scorePoint;
				ufoCX = cx;
				ufoCY = cy;
			}

			if(dPlayer >= minPlayerDist && dEarth >= minEarthUfoDist)
			{
				Ponto p;
				p.x = cx;
				p.y = cy;
				candidatos.push_back(p);
			}
		}

	if(!candidatos.empty())
	{
		int escolhido = rand() % (int)candidatos.size();
		ufoCX = candidatos[escolhido].x;
		ufoCY = candidatos[escolhido].y;
	}

	// FollowerObject usa o centro da hitbox para se mover; manter o centro
	// no ponto sorteado preserva o sprite 96x96 centralizado.
	ufo->x = ufoCX - ufo->w/2;
	ufo->y = ufoCY - ufo->h/2;
}

void StateGame::buildTurrets()
{
	if(!tileMap || !ship || !ufo)
		return;

	int maxTorres = GameSettings::turretCount();
	if(maxTorres <= 0)
		return;

	// Candidatas: centro dos paineis solidos (camada 1), longe da nave,
	// da Terra (quando existe) e do UFO. A distancia minima entre torres
	// e checada na escolha.
	struct Ponto { float x, y; };
	std::vector<Ponto> candidatas;

	for(int tx = 0; tx < tileMap->width(); ++tx)
		for(int ty = 0; ty < tileMap->height(); ++ty)
		{
			if(tileMap->at(tx, ty, 1) == 0)
				continue;

			float cx = tx * TILE_SIZE + TILE_SIZE / 2.0f;
			float cy = ty * TILE_SIZE + TILE_SIZE / 2.0f;

			if(distanceBetween(cx, cy, ship->x + ship->w/2, ship->y + ship->h/2) < 480.0f)
				continue;
			if(earth && distanceBetween(cx, cy, earth->x + earth->w/2, earth->y + earth->h/2) < 260.0f)
				continue;
			if(distanceBetween(cx, cy, ufo->x + ufo->w/2, ufo->y + ufo->h/2) < 260.0f)
				continue;

			Ponto p;
			p.x = cx;
			p.y = cy;
			candidatas.push_back(p);
		}

	int escolhas = 0;
	int tentativas = 0;
	while(escolhas < maxTorres && !candidatas.empty() && tentativas < 300)
	{
		++tentativas;
		int idx = rand() % (int)candidatas.size();
		float cx = candidatas[idx].x;
		float cy = candidatas[idx].y;

		bool pertoDemais = false;
		for(std::vector<Turret*>::iterator t = turretArray.begin(); t != turretArray.end(); ++t)
		{
			if(distanceBetween(cx, cy, (*t)->x + (*t)->w/2, (*t)->y + (*t)->h/2) < 200.0f)
			{
				pertoDemais = true;
				break;
			}
		}

		if(pertoDemais)
		{
			candidatas.erase(candidatas.begin() + idx);
			continue;
		}

		turretArray.push_back(new Turret(new Sprite("img/Torreta.png"), cx - 16.0f, cy - 16.0f));
		candidatas.erase(candidatas.begin() + idx);
		++escolhas;
	}
}

void StateGame::spawnInitialPlanets()
{
	if((!redPlanetSprite && !purplePlanetSprite) || !ship || !ufo)
		return;

	float shipCX = ship->x + ship->w/2;
	float shipCY = ship->y + ship->h/2;
	float earthCX = earth ? earth->x + earth->w/2 : -1e9f;
	float earthCY = earth ? earth->y + earth->h/2 : -1e9f;
	float ufoCX = ufo->x + ufo->w/2;
	float ufoCY = ufo->y + ufo->h/2;

	// Pequena quantidade inicial: 2 a 4 planetas ja posicionados no mapa.
	int quantidade = 2 + rand() % 3;

	for(int i = 0; i < quantidade; ++i)
	{
		// O tipo e sorteado antes: planeta roxo (mais lento, 4 tiros) ou
		// vermelho (2 tiros). Cada um varre a grade com suas proprias medidas.
		bool roxo = (rand() % 100 < 25);
		Sprite *sprite = roxo ? purplePlanetSprite : redPlanetSprite;
		float planetW = sprite->getRenderWidth();
		float planetH = sprite->getRenderHeight();

		// Tambem usa uma grade de posicoes validas: garante que os planetas
		// iniciais nao nascam em cima da nave, da Terra, do UFO, dentro de
		// um painel solido nem uns dos outros, e sorteia entre as opcoes.
		struct Ponto { float x, y; };
		std::vector<Ponto> candidatos;

		float bestX = 0.0f;
		float bestY = 0.0f;
		float bestScore = -1.0f;
		const float passo = 20.0f;

		for(float x = 0; x <= worldWidth - planetW; x += passo)
			for(float y = 0; y <= worldHeight - planetH; y += passo)
			{
				float cx = x + planetW / 2;
				float cy = y + planetH / 2;

				float dPlayer = distanceBetween(cx, cy, shipCX, shipCY);
				float dEarth = distanceBetween(cx, cy, earthCX, earthCY);
				float dUfo = distanceBetween(cx, cy, ufoCX, ufoCY);
				float dOther = 1e9f;

				for(std::vector<Planet*>::iterator it = planetArray.begin(); it != planetArray.end(); ++it)
				{
					float d = distanceBetween(cx, cy, (*it)->x + (*it)->w/2, (*it)->y + (*it)->h/2);
					if(d < dOther)
						dOther = d;
				}

				float scorePoint = dPlayer;
				if(dEarth < scorePoint) scorePoint = dEarth;
				if(dUfo < scorePoint) scorePoint = dUfo;
				if(dOther < scorePoint) scorePoint = dOther;

				if(scorePoint > bestScore)
				{
					bestScore = scorePoint;
					bestX = x;
					bestY = y;
				}

				if(dPlayer >= 280.0f && dEarth >= 220.0f &&
				   dUfo >= 170.0f && dOther >= 120.0f &&
				   !tileMap->solidAt(cx, cy))
				{
					Ponto p;
					p.x = x;
					p.y = y;
					candidatos.push_back(p);
				}
			}

		if(!candidatos.empty())
		{
			int escolhido = rand() % (int)candidatos.size();
			bestX = candidatos[escolhido].x;
			bestY = candidatos[escolhido].y;
		}

		// Vermelho: 2 tiros, 10 pontos, 20% de drop. Roxo: 4 tiros, 25
		// pontos, deriva 55% mais lenta e 60% de drop.
		planetArray.push_back(new PlanetRed(sprite, bestX, bestY, roxo ? 4 : 2,
		                                    (float)worldWidth, (float)worldHeight,
		                                    roxo ? 25 : 10, roxo ? 60 : 20,
		                                    roxo ? 0.55f : 1.0f, tileMap,
		                                    roxo ? 8 : 5));
	}
}

// ------------------------------------------------------------------ pontuacao

void StateGame::addKillScore(int base, float x, float y)
{
	++comboStreak;
	if(comboStreak > bestStreak)
		bestStreak = comboStreak;
	// Janela maior no primeiro minuto (4 s): o spawn ainda e lento e o
	// combo seria impossivel de emendar. Depois volta aos 3 s normais.
	comboTimer = (gameTimeMs < 60000) ? 4000 : (int)COMBO_WINDOW_MS;

	// x1.0 no primeiro abate; +0.25 por abate emendado, ate o teto de x3.
	comboMult = 1.0f + 0.25f * (comboStreak - 1);
	if(comboMult > 3.0f)
		comboMult = 3.0f;

	int pontos = (int)(base * comboMult);
	score += pontos;

	char buf[24];
	snprintf(buf, sizeof(buf), "+%d", pontos);
	addFloater(buf, x, y, makeColor(255, 230, 130), false, 900);

	if(comboStreak >= 8)
		tryAch(ACH_COMBO);
}

void StateGame::spawnDrop(float x, float y, int chancePercent)
{
	if(rand() % 100 >= chancePercent)
		return;

	// A fila de drops nao passa de 4 intens na tela.
	if(powerUpArray.size() >= 4)
		return;

	int kind = rand() % 4;
	int pw = powerUpSprites[kind]->getRenderWidth();
	int ph = powerUpSprites[kind]->getRenderHeight();

	float px = clampi((int)x, 0, worldWidth - pw);
	float py = clampi((int)y, 0, worldHeight - ph);

	powerUpArray.push_back(new PowerUp(powerUpSprites[kind], px, py, kind));
}

void StateGame::tryAch(int id)
{
	if(PlayStats::unlockAch(id))
	{
		toast("CONQUISTA: " + std::string(PlayStats::achName(id)));
		if(sfxPickup)
			sfxPickup->play(0);
	}
}

void StateGame::checkAchievements()
{
	// Conquistas avaliadas durante a partida (as de fim de jogo ficam no endGame).
	if(gameTimeMs >= 300000)
		tryAch(ACH_5MIN);

	if(PlayStats::totalPlanets() + destroyedCount >= 50)
		tryAch(ACH_50);
}

// --------------------------------------------------------------------- chefe

void StateGame::updateUfoBehavior(int dt)
{
	// Explosao em cadeia do chefe derrotado: diversos "booms" em volta do
	// ponto da morte, com tremor longo.
	if(chainTimer > 0)
	{
		chainTimer -= dt;
		chainTick -= dt;
		if(chainTick <= 0 && chainTimer > 0)
		{
			chainTick = 170;
			boomX = chainX + (rand() % 90) - 45;
			boomY = chainY + (rand() % 90) - 45;
			boomActive = true;
			boom->restart();
			sfx->play(0);
			if(shakeTime < 300)
				shakeTime = 300;
		}
	}

	// Chefe morto: conta a pausa e poe a proxima onda no lugar.
	if(!ufo)
	{
		if(ufoRespawnTimer > 0)
		{
			ufoRespawnTimer -= dt;
			if(ufoRespawnTimer <= 0)
				respawnUfo();
		}
		return;
	}

	if(!ship)
		return;

	// Fases por vida restante: 1 persegue, 2 atira, 3 atira em leque e
	// invoca planetas. Cada fase tambem acelera a perseguinte, mas o teto
	// fica abaixo da velocidade base da nave (0.35): pressao sem tornar a
	// fuga impossivel sem turbo.
	int fase = (ufo->hp * 3 > ufo->maxHp * 2) ? 1 : (ufo->hp * 3 > ufo->maxHp) ? 2 : 3;
	float chase = 0.22f + 0.14f * (gameTimeMs / 120000.0f);
	if(chase > 0.36f)
		chase = 0.36f;
	chase += 0.03f * (wave - 1);            // ondas sucessivas sao mais rapidas
	chase *= (fase == 2) ? 1.1f : (fase == 3) ? 1.22f : 1.0f;
	chase *= GameSettings::chaseMult();
	if(chase > 0.33f)
		chase = 0.33f;
	ufo->setChaseSpeed(chase);
	ufo->update(dt);

	float ux = ufo->x + ufo->w/2;
	float uy = ufo->y + ufo->h/2;
	float sx = ship->x + ship->w/2;
	float sy = ship->y + ship->h/2;

	if(fase >= 2)
	{
		ufoShootTimer -= dt;
		if(ufoShootTimer <= 0)
		{
			if(fase == 3)
			{
				// Leque de 3 tiros (dano da dificuldade).
				int danoTiro = GameSettings::enemyShotDamage();
				addEnemyShot(ux, uy, sx, sy, 0.30f, danoTiro, -20);
				addEnemyShot(ux, uy, sx, sy, 0.30f, danoTiro, 0);
				addEnemyShot(ux, uy, sx, sy, 0.30f, danoTiro, 20);
				ufoShootTimer = (int)(1600 * GameSettings::enemyShotMult());
			}
			else
			{
				addEnemyShot(ux, uy, sx, sy, 0.28f, GameSettings::enemyShotDamage(), 0);
				ufoShootTimer = (int)(2300 * GameSettings::enemyShotMult());
			}
		}
	}
	else
		ufoShootTimer = (int)(2300 * GameSettings::enemyShotMult());

	if(fase == 3)
	{
		ufoSummonTimer -= dt;
		if(ufoSummonTimer <= 0)
		{
			if(planetArray.size() < 14)
				addPlanet();
			ufoSummonTimer = 12000;
		}
	}
	else
		ufoSummonTimer = 12000;
}

void StateGame::killUfo()
{
	if(!ufo)
		return;

	float ux = ufo->x + ufo->w/2;
	float uy = ufo->y + ufo->h/2;

	addKillScore(300, ux, uy);
	toast("UFO DESTRUÍDO!");
	++ufosKilledThisRun;
	++wave;
	tryAch(ACH_HUNT);

	// Explosao em cadeia no ponto da morte; o proximo UFO chega em ~6,5 s.
	chainTimer = 800;
	chainTick = 0;
	chainX = ux;
	chainY = uy;
	if(shakeTime < 700)
		shakeTime = 700;

	// Espolio do chefe: combustivel + escudo garantidos no ponto da morte
	// (vira decisao risco/recompensa cacar o UFO). Respeita um teto de 6
	// itens para nao lotar a tela junto da agenda e dos drops.
	int tipos[2] = {PowerUp::FUEL, PowerUp::ESCUDO};
	for(int i = 0; i < 2; ++i)
	{
		if(powerUpArray.size() >= 6)
			break;
		int pw = powerUpSprites[tipos[i]]->getRenderWidth();
		int ph = powerUpSprites[tipos[i]]->getRenderHeight();
		float px = clampi((int)(ux + (i == 0 ? -40.0f : 40.0f)), 0, worldWidth - pw);
		float py = clampi((int)uy, 0, worldHeight - ph);
		powerUpArray.push_back(new PowerUp(powerUpSprites[tipos[i]], px, py, tipos[i]));
	}
	addFloater("RECOMPENSA!", ux, uy - 60.0f, makeColor(130, 255, 160), false, 1200);

	delete(ufo);
	ufo = NULL;
	hud->setBossLife(0, 0, wave);
	ufoRespawnTimer = 6500;
}

void StateGame::respawnUfo()
{
	if(!ship)
		return;

	ufo = new FollowerObject(ufoSprite, 0, 0);
	ufo->setHitbox(80, 48);
	ufo->setAutonomous(true);
	ufo->setChaseTarget(ship);
	ufo->maxHp = GameSettings::ufoBaseHp() + (wave - 1) * GameSettings::ufoHpPerWave();
	ufo->hp = ufo->maxHp;

	// Nasce longe da nave e da Terra (sem a restricao mutua da partida
	// inicial, que ja nao faz sentido com o chefe em movimento).
	float shipCX = ship->x + ship->w/2;
	float shipCY = ship->y + ship->h/2;
	float earthCX = earth ? earth->x + earth->w/2 : worldWidth / 2.0f;
	float earthCY = earth ? earth->y + earth->h/2 : worldHeight / 2.0f;
	float diagonal = sqrtf((float)(worldWidth * worldWidth + worldHeight * worldHeight));
	float minPlayer = diagonal * 0.45f;
	float minEarth = diagonal * 0.30f;

	struct Ponto { float x, y; };
	std::vector<Ponto> candidatos;

	float ufoHalf = ufoSprite->getRenderWidth() / 2.0f;
	float ufoMargin = ufoHalf + 4.0f;
	if(ufoMargin > worldWidth / 2)
		ufoMargin = worldWidth / 2;
	if(ufoMargin > worldHeight / 2)
		ufoMargin = worldHeight / 2;

	float melhorCX = worldWidth / 2.0f;
	float melhorCY = worldHeight / 2.0f;
	float melhorNota = -1.0f;
	const float passo = 40.0f;

	for(float cx = ufoMargin; cx <= worldWidth - ufoMargin; cx += passo)
		for(float cy = ufoMargin; cy <= worldHeight - ufoMargin; cy += passo)
		{
			float dPlayer = distanceBetween(cx, cy, shipCX, shipCY);
			float dEarth = distanceBetween(cx, cy, earthCX, earthCY);
			float nota = dPlayer < dEarth ? dPlayer : dEarth;

			if(nota > melhorNota)
			{
				melhorNota = nota;
				melhorCX = cx;
				melhorCY = cy;
			}
			if(dPlayer >= minPlayer && dEarth >= minEarth)
			{
				Ponto p;
				p.x = cx;
				p.y = cy;
				candidatos.push_back(p);
			}
		}

	if(!candidatos.empty())
	{
		int escolhido = rand() % (int)candidatos.size();
		melhorCX = candidatos[escolhido].x;
		melhorCY = candidatos[escolhido].y;
	}

	ufo->x = melhorCX - ufo->w/2;
	ufo->y = melhorCY - ufo->h/2;
	hud->setBossLife(ufo->hp, ufo->maxHp, wave);

	toast("ONDA " + std::to_string(wave) + ": o UFO voltou mais forte!");
}

// -------------------------------------------------------------------- torres

void StateGame::updateTurrets(int dt)
{
	for(std::vector<Turret*>::iterator it = turretArray.begin(); it != turretArray.end(); ++it)
	{
		Turret *t = *it;
		t->update(dt);

		if(!ship)
			continue;

		float tcx = t->x + t->w/2;
		float tcy = t->y + t->h/2;
		float scx = ship->x + ship->w/2;
		float scy = ship->y + ship->h/2;

		t->aimAt(scx, scy);

		if(t->cooldown <= 0 && distanceBetween(tcx, tcy, scx, scy) < 560.0f &&
		   lineIsClear(tcx, tcy, scx, scy))
		{
			addEnemyShot(tcx, tcy, scx, scy, 0.34f, GameSettings::enemyShotDamage(), 0);
			t->cooldown = (int)(2300 * GameSettings::enemyShotMult());
		}
	}
}

// Linha de visada: amostra o caminho tiro a tiro; qualquer painel solido no
// meio bloqueia. Comeca depois da borda do tile da origem (40 px >= metade
// do tile + folga) para o atirador nao "enxergar" o proprio painel, e os
// ultimos 40 px (chegada no alvo) nao contam.
bool StateGame::lineIsClear(float x0, float y0, float x1, float y1)
{
	if(!tileMap)
		return true;

	float dx = x1 - x0;
	float dy = y1 - y0;
	float dist = sqrtf(dx*dx + dy*dy);
	if(dist < 80.0f)
		return true;

	for(float p = 40.0f; p < dist - 40.0f; p += 14.0f)
	{
		float t = p / dist;
		if(tileMap->solidAt(x0 + dx * t, y0 + dy * t))
			return false;
	}

	return true;
}

void StateGame::addEnemyShot(float fromX, float fromY, float targetX, float targetY,
                             float speed, int damage, int spreadDeg)
{
	float ang = atan2f(targetY - fromY, targetX - fromX);
	ang += spreadDeg * PI / 180.0f;

	Bullet b;
	b.x = fromX;
	b.y = fromY;
	b.vx = cosf(ang) * speed;
	b.vy = sinf(ang) * speed;
	b.life = 3500;
	b.damage = damage;
	b.grace = 220;            // o tiro sai do painel/tile de origem
	b.enemy = true;
	bulletArray.push_back(b);

	++enemyShotsFired;
}

// ------------------------------------------------------------- textos flutuantes

void StateGame::addFloater(string text, float x, float y, SDL_Color color, bool big, int lifeMs)
{
	int alvo = -1;

	// Primeiro slot livre do tamanho pedido; senao reutiliza o mais antigo.
	for(std::vector<Floater>::size_type i = 0; i < floaters.size(); ++i)
	{
		if(floaters[i].big != big)
			continue;
		if(!floaters[i].active)
		{
			alvo = i;
			break;
		}
		if(alvo == -1 || floaters[i].life < floaters[alvo].life)
			alvo = i;
	}

	if(alvo == -1)
		return; // pool vazio (so acontece sem slots do tipo, o que nao existe)

	Floater &f = floaters[alvo];
	f.active = true;
	f.text->setText(text);
	f.text->setColor(color);
	f.x = x;
	f.y = y;
	f.life = lifeMs;
	f.total = lifeMs;
}

void StateGame::toast(string text)
{
	// Aviso grande acima da nave (posicao no mundo; acompanha a camera).
	float x = ship ? ship->x + ship->w/2 : worldWidth / 2.0f;
	float y = ship ? ship->y - 130.0f : 120.0f;
	addFloater(text, x, y, makeColor(120, 240, 255), true, 2600);
}

void StateGame::updateFloaters(int dt)
{
	for(std::vector<Floater>::size_type i = 0; i < floaters.size(); ++i)
	{
		Floater &f = floaters[i];
		if(!f.active)
			continue;

		f.life -= dt;
		if(f.life <= 0)
		{
			f.active = false;
			f.text->setText("");
			continue;
		}

		// Sobe devagar enquanto vive.
		f.y -= (f.big ? 0.008f : 0.035f) * dt;
	}
}

void StateGame::renderFloaters(float cameraX, float cameraY)
{
	for(std::vector<Floater>::size_type i = 0; i < floaters.size(); ++i)
	{
		Floater &f = floaters[i];
		if(!f.active)
			continue;

		int sx = (int)(f.x - cameraX - f.text->getWidth() / 2);
		int sy = (int)(f.y - cameraY);

		if(sx > SCREEN_W || sy > SCREEN_H || sx + f.text->getWidth() < 0 || sy + f.text->getHeight() < 0)
			continue;

		f.text->setPos(sx, sy);
		f.text->render();
	}
}

void StateGame::updateTutorial()
{
	if(!ship || returnState == 3)
		return;

	// Cada dica aparece uma vez, so para quem ainda nao fez a acao.
	if(!tutThrustShown && !tutThrustDone && gameTimeMs > 4000)
	{
		tutThrustShown = true;
		toast("W/S: impulso e freio");
	}
	if(!tutTurnShown && !tutTurnDone && gameTimeMs > 9000)
	{
		tutTurnShown = true;
		toast("A/D: gira a nave");
	}
	if(!tutShotShown && !tutShotDone && gameTimeMs > 14000)
	{
		tutShotShown = true;
		toast("ESPACO/MOUSE1: atira");
	}
	if(!tutEarthShown && gameTimeMs > 21000)
	{
		tutEarthShown = true;
		if(GameSettings::gameMode() == MODE_PTS)
			toast("Sobreviva ate o fim!");
		else
			toast("Siga a seta amarela ate a Terra");
	}
}

// ------------------------------------------------------------------ combustivel

void StateGame::updateFuel(int dt)
{
	if(!ship)
		return;

	fuel -= GameSettings::fuelDrainPerSec() * dt / 1000.0f;
	if(fuel < 0.0f)
		fuel = 0.0f;

	hud->setFuel((int)fuel);

	if(fuel > 5.0f)
	{
		fuelAnnounced = false;
		fuelDamageTimer = 0;
	}
	else
		fuelLowAnnounced = true; // ja vazio: nao avisa "baixo" depois

	// Pre-aviso aos 25%: da tempo de cacar um cristal amarelo.
	if(fuel <= 25.0f && fuel > 5.0f && !fuelLowAnnounced)
	{
		fuelLowAnnounced = true;
		toast("COMBUSTIVEL BAIXO!");
	}
	if(fuel > 30.0f)
		fuelLowAnnounced = false;

	if(fuel > 5.0f)
		return;

	// Tanque vazio: a nave perde 3 de vida por segundo.
	if(!fuelAnnounced)
	{
		fuelAnnounced = true;
		toast("SEM COMBUSTIVEL!");
	}

	fuelDamageTimer += dt;
	if(fuelDamageTimer >= 1000)
	{
		fuelDamageTimer -= 1000;
		ship->hp -= 3;
		hud->setLife(ship->hp);
		tookDamage = true;
		addFloater("-3", ship->x + ship->w/2, ship->y, makeColor(255, 90, 90), false, 800);
	}
}

void StateGame::updateSurvivalScore(int dt)
{
	// Sobreviver tambem pontua: +10 a cada 10 s.
	survivalTimer += dt;
	while(survivalTimer >= 10000)
	{
		survivalTimer -= 10000;
		score += 10;
	}
}

int StateGame::unload()
{
	delete(bg);
	delete(tileMap);
	delete(tileSet);
	delete(redPlanetSprite);
	delete(purplePlanetSprite);
	delete(mineSprite);
	delete(earth);
	delete(earthSprite);
	delete(moon);
	delete(moonSprite);

	for(std::vector<Planet*>::iterator it = planetArray.begin(); it != planetArray.end(); ++it)
		delete(*it);
	planetArray.clear();

	for(std::vector<Mine*>::iterator it = mineArray.begin(); it != mineArray.end(); ++it)
		delete(*it);
	mineArray.clear();

	for(std::vector<Turret*>::iterator it = turretArray.begin(); it != turretArray.end(); ++it)
		delete(*it);
	turretArray.clear();

	for(std::vector<PowerUp*>::iterator it = powerUpArray.begin(); it != powerUpArray.end(); ++it)
		delete(*it);
	powerUpArray.clear();

	for(std::vector<Floater>::size_type i = 0; i < floaters.size(); ++i)
		delete(floaters[i].text);
	floaters.clear();

	delete(powerUpSprites[0]);
	delete(powerUpSprites[1]);
	delete(powerUpSprites[2]);
	delete(powerUpSprites[3]);

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
	Camera::getInstance().clearBounds();

	return returnStack;
}

void StateGame::checkCollision(int dt)
{
	// Power-ups: coleta (so ocorre com a nave viva).
	for(std::vector<PowerUp*>::iterator it = powerUpArray.begin(); it != powerUpArray.end(); )
	{
		if(ship && (*it)->collidesWith(ship))
		{
			char msg[32];
			switch((*it)->kind)
			{
				case PowerUp::VIDA:
					ship->hp = (ship->hp + 20 > 100) ? 100 : ship->hp + 20;
					snprintf(msg, sizeof(msg), "+20 VIDA");
					break;
				case PowerUp::ESCUDO:
					ship->setShield(6000);
					snprintf(msg, sizeof(msg), "ESCUDO!");
					break;
				case PowerUp::TURBO:
					ship->setBoost(BOOST_MULT, BOOST_TIME);
					snprintf(msg, sizeof(msg), "TURBO!");
					break;
				default: // FUEL
					fuel += 40.0f;
					if(fuel > 100.0f)
						fuel = 100.0f;
					snprintf(msg, sizeof(msg), "+40%% COMB.");
					break;
			}
			hud->setLife(ship->hp);
			sfxPickup->play(0);
			addFloater(msg, (*it)->x + (*it)->w/2, (*it)->y, makeColor(130, 255, 160), false, 1000);

			delete(*it);
			it = powerUpArray.erase(it);
		}
		else
			++it;
	}

	if(ship == NULL)
		return;

	float mult = GameSettings::damageMult();
	float scx = ship->x + ship->w/2;
	float scy = ship->y + ship->h/2;

	// Tiros inimigos (chefe/torres): o escudo os absorve; os i-frames fazem
	// atravessarem sem dano repetido.
	std::vector<Bullet>::iterator b = bulletArray.begin();
	while(b != bulletArray.end())
	{
		if(b->enemy && bulletHit(*b, ship))
		{
			if(ship->getShieldTime() > 0)
			{
				b = bulletArray.erase(b);
				continue;
			}
			if(invulnTime <= 0)
			{
				ship->hp -= b->damage;
				hud->setLife(ship->hp);
				sfx->play(0);
				invulnTime = 800;
				shakeTime = 250;
				tookDamage = true;

				char buf[16];
				snprintf(buf, sizeof(buf), "-%d", b->damage);
				addFloater(buf, scx, scy, makeColor(255, 90, 90), false, 900);

				b = bulletArray.erase(b);
				continue;
			}
		}
		++b;
	}

	// Planetas: dano do tipo (vermelho 5, roxo 8), com i-frames (durante
	// eles a nave atravessa o planeta sem destrui-lo).
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

				PlanetRed *p = (PlanetRed*)(*it);
				int dano = (int)(p->dano * mult);
				ship->hp -= dano;
				hud->setLife(ship->hp);
				destroyedCount++;
				// Abalroar tambem pontua (metade do valor, arredondando para
				// cima) e pode dropar: o kill por corpo entra no combo como
				// o kill por tiro, sem confundir o jogador.
				addKillScore((p->valor + 1) / 2, (*it)->x + (*it)->w/2, (*it)->y);
				spawnDrop((*it)->x + (*it)->w/2, (*it)->y + (*it)->h/2, p->dropChance);
				sfx->play(0);
				invulnTime = 800;
				shakeTime = 250;
				tookDamage = true;

				char buf[16];
				snprintf(buf, sizeof(buf), "-%d", dano);
				addFloater(buf, scx, scy, makeColor(255, 90, 90), false, 900);

				delete(*it);
				it = planetArray.erase(it);
			}
			else
				++it;
		}
		else
			++it;
	}

	// Minas: armam a espoleta (600 ms) a 110 px da nave e o contato
	// direto detona na hora. A explosao fica para a passada propria.
	for(std::vector<Mine*>::iterator it = mineArray.begin(); it != mineArray.end(); ++it)
	{
		float mdx = (*it)->x + (*it)->w/2 - scx;
		float mdy = (*it)->y + (*it)->h/2 - scy;
		if(mdx*mdx + mdy*mdy < 110.0f * 110.0f && (*it)->fuse < 0)
		{
			(*it)->fuse = 600;
			addFloater("!", (*it)->x + (*it)->w/2, (*it)->y, makeColor(255, 90, 90), false, 600);
		}
		if((*it)->collidesWith(ship))
			(*it)->fuse = 0;
	}

	// Detonacoes pendentes (a cadeia arma as vizinhas dentro do detonate).
	// Apos cada erase o vetor desloca e o end() encolhe: revalida do
	// inicio a cada remocao (sao no maximo 4 minas, e cada detonacao remove
	// uma, entao sempre termina). Sem isso o iterador avancado antes do
	// erase passa do novo fim e le lixo alem do vetor.
	for(std::vector<Mine*>::iterator it = mineArray.begin(); it != mineArray.end(); )
	{
		if((*it)->fuse == 0)
		{
			std::vector<Mine*>::iterator vitima = it;
			detonateMine(vitima); // apaga vitima do vetor
			it = mineArray.begin(); // revalida (o erase invalidou o resto)
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

		int dano = (int)(MOON_COLLISION_DAMAGE * mult);
		ship->hp -= dano;
		hud->setLife(ship->hp);
		sfx->play(0);
		invulnTime = 800;
		shakeTime = 250;
		tookDamage = true;

		char buf[16];
		snprintf(buf, sizeof(buf), "-%d", dano);
		addFloater(buf, scx, scy, makeColor(255, 90, 90), false, 900);
	}

	// UFO: encostar custa 30 de vida (30% do maximo), com os mesmos i-frames
	// dos planetas/lua. O escudo evita o dano enquanto estiver ativo.
	if(ufo && ship->collidesWith(ufo) && invulnTime <= 0 && ship->getShieldTime() <= 0)
	{
		boomX = ufo->x + ufo->w/2;
		boomY = ufo->y + ufo->h/2;
		boomActive = true;
		boom->restart();

		int dano = (int)(UFO_COLLISION_DAMAGE * mult);
		ship->hp -= dano;
		hud->setLife(ship->hp);
		sfx->play(0);
		invulnTime = 800;
		shakeTime = 250;
		tookDamage = true;

		char buf[16];
		snprintf(buf, sizeof(buf), "-%d", dano);
		addFloater(buf, scx, scy, makeColor(255, 90, 90), false, 900);
	}

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

void StateGame::shoot()
{
	if(ship == NULL)
		return;

	tutShotDone = true; // tutorial: o jogador ja sabe atirar

	// Direcao do bico da nave (mesma rotacao usada por AccObject::update:
	// rotacao 0 aponta para cima, +90 para a esquerda).
	float angulo = (ship->getRotation() - 90.0f) * PI / 180.0f;
	float dirX = -cosf(angulo);
	float dirY = sinf(angulo);

	float cx = ship->x + ship->w/2;
	float cy = ship->y + ship->h/2;

	Bullet bullet;
	bullet.x = cx + dirX * 58.0f;
	bullet.y = cy + dirY * 58.0f;
	bullet.vx = dirX * 0.95f; // px/ms
	bullet.vy = dirY * 0.95f;
	bullet.life = 1300;       // alcance de ~1235 px
	bullet.damage = 1;
	bullet.grace = 60;        // ~57 px: sai do cano sem "furar" a parede ao lado
	bullet.enemy = false;
	bulletArray.push_back(bullet);
}

bool StateGame::bulletHit(const Bullet &bullet, GameObject *object)
{
	const float raio = 4.0f;

	return bullet.x + raio >= object->x &&
	       bullet.x - raio <= object->x + object->w &&
	       bullet.y + raio >= object->y &&
	       bullet.y - raio <= object->y + object->h;
}

void StateGame::updateBullets(int dt)
{
	std::vector<Bullet>::iterator it = bulletArray.begin();
	while(it != bulletArray.end())
	{
		it->x += it->vx * dt;
		it->y += it->vy * dt;
		it->life -= dt;

		bool remove = it->life <= 0;

		// Nenhum tiro atravessa painel solido: os dois lados usam o mapa
		// como cobertura. O "grace" inicial impede que o tiro morra no
		// proprio painel de origem (torre dentro do tile / nave encostada).
		// Tiro do jogador que morre no painel deixa uma faisca ("x") para
		// o bloqueio ficar visivel.
		if(it->grace > 0)
			it->grace -= dt;
		else if(!remove && tileMap && tileMap->solidAt(it->x, it->y))
		{
			if(!it->enemy)
				addFloater("x", it->x, it->y, makeColor(255, 200, 110), false, 300);
			remove = true;
		}

		if(!remove && !it->enemy)
		{
			// Planetas: cada projetil tira 1 de vida (vermelho aguenta 2,
			// roxo 4). Morte rende pontos com combo e possivel drop.
			for(std::vector<Planet*>::iterator p = planetArray.begin(); p != planetArray.end(); ++p)
			{
				if(bulletHit(*it, *p))
				{
					(*p)->hp -= 1;
					if((*p)->hp <= 0)
					{
						boomX = (*p)->x + (*p)->w/2;
						boomY = (*p)->y + (*p)->h/2;
						boomActive = true;
						boom->restart();
						sfx->play(0);
						destroyedCount++;

						PlanetRed *pr = (PlanetRed*)(*p);
						addKillScore(pr->valor, pr->x + pr->w/2, pr->y);
						spawnDrop(pr->x + pr->w/2, pr->y + pr->h/2, pr->dropChance);

						delete(*p);
						planetArray.erase(p);
					}

					remove = true;
					break;
				}
			}
		}

		if(!remove && !it->enemy)
		{
			// Torres: 2 tiros, rendem 30 e dropam 40%.
			for(std::vector<Turret*>::iterator t = turretArray.begin(); t != turretArray.end(); ++t)
			{
				if(bulletHit(*it, *t))
				{
					(*t)->hp -= 1;
					if((*t)->hp <= 0)
					{
						boomX = (*t)->x + (*t)->w/2;
						boomY = (*t)->y + (*t)->h/2;
						boomActive = true;
						boom->restart();
						sfx->play(0);
						addKillScore(30, (*t)->x + (*t)->w/2, (*t)->y);
						spawnDrop((*t)->x + (*t)->w/2, (*t)->y + (*t)->h/2, 40);

						delete(*t);
						turretArray.erase(t);
					}

					remove = true;
					break;
				}
			}
		}

		// Minas: 1 tiro, 15 pontos e 25% de drop.
	if(!remove && !it->enemy)
	{
		for(std::vector<Mine*>::iterator m = mineArray.begin(); m != mineArray.end(); ++m)
		{
			if(bulletHit(*it, *m))
			{
				boomX = (*m)->x + (*m)->w/2;
				boomY = (*m)->y + (*m)->h/2;
				boomActive = true;
				boom->restart();
				sfx->play(0);
				destroyedCount++;

				addKillScore(15, (*m)->x + (*m)->w/2, (*m)->y);
				spawnDrop((*m)->x + (*m)->w/2, (*m)->y + (*m)->h/2, 25);

				delete(*m);
				mineArray.erase(m);

				remove = true;
				break;
			}
		}
	}

	// UFO: chefe com HP escalonado. Derrota-lo abre a proxima onda.
		if(!remove && !it->enemy && ufo && bulletHit(*it, ufo))
		{
			ufo->hp -= 1;
			if(ufo->hp <= 0)
				killUfo();

			remove = true;
		}

		if(remove)
			it = bulletArray.erase(it);
		else
			++it;
	}
}

// Reversao do movimento da nave ao bater num painel solido: a posicao volta
// para o frame anterior e a velocidade "quica" (meia forca, sentido oposto).
void StateGame::resolveShipTiles()
{
	if(!ship || !tileMap)
		return;

	const float m = 10.0f; // margem interna dos cantos amostrados
	float cx = ship->x + ship->w / 2;
	float cy = ship->y + ship->h / 2;

	bool bate = tileMap->solidAt(ship->x + m, ship->y + m) ||
	            tileMap->solidAt(ship->x + ship->w - m, ship->y + m) ||
	            tileMap->solidAt(ship->x + m, ship->y + ship->h - m) ||
	            tileMap->solidAt(ship->x + ship->w - m, ship->y + ship->h - m) ||
	            tileMap->solidAt(cx, ship->y + m) ||
	            tileMap->solidAt(cx, ship->y + ship->h - m) ||
	            tileMap->solidAt(ship->x + m, cy) ||
	            tileMap->solidAt(ship->x + ship->w - m, cy);

	if(!bate)
		return;

	float velAnterior = ship->getSpeed();
	ship->x = shipPrevX;
	ship->y = shipPrevY;

	if(velAnterior > 0.03f || velAnterior < -0.03f)
	{
		ship->setSpeed(-velAnterior * 0.5f);
		if(velAnterior > 0.12f || velAnterior < -0.12f)
		{
			if(shakeTime < 150)
				shakeTime = 150;
		}
	}
	else
		ship->setSpeed(0.0f);
}

void StateGame::addPlanet()
{
	if(ship == NULL)
		return;

	float sx = ship->x + ship->w/2;
	float sy = ship->y + ship->h/2;

	// 25% dos novos planetas sao roxos: mais resistentes, mais lentos,
	// mais valiosos e com drop quase garantido.
	bool roxo = (rand() % 100 < 25);
	Sprite *sprite = roxo ? purplePlanetSprite : redPlanetSprite;

	int planetW = sprite->getRenderWidth();
	int planetH = sprite->getRenderHeight();

	// Intersecao entre a tela atual e o mapa: o planeta nasce visivel e
	// nunca fora dos limites do mundo aleatorio.
	int areaMinX = (int)Camera::getInstance().getX();
	int areaMaxX = areaMinX + SCREEN_W - planetW;
	int areaMinY = (int)Camera::getInstance().getY();
	int areaMaxY = areaMinY + SCREEN_H - planetH;
	if(areaMinX < 0)
		areaMinX = 0;
	if(areaMinY < 0)
		areaMinY = 0;
	if(areaMaxX > worldWidth - planetW)
		areaMaxX = worldWidth - planetW;
	if(areaMaxY > worldHeight - planetH)
		areaMaxY = worldHeight - planetH;
	if(areaMaxX < areaMinX || areaMaxY < areaMinY)
	{
		areaMinX = 0;
		areaMaxX = worldWidth - planetW;
		areaMinY = 0;
		areaMaxY = worldHeight - planetH;
	}

	// Posicao aleatoria na tela, tentando nascer longe da nave e fora de
	// paineis solidos (dentro de um bloco o planeta nasceria "preso").
	int planetX = areaMinX + rand() % (areaMaxX - areaMinX + 1);
	int planetY = areaMinY + rand() % (areaMaxY - areaMinY + 1);
	for(int tentativa = 0; tentativa < 20; ++tentativa)
	{
		planetX = areaMinX + rand() % (areaMaxX - areaMinX + 1);
		planetY = areaMinY + rand() % (areaMaxY - areaMinY + 1);

		float dx = planetX + planetW/2 - sx;
		float dy = planetY + planetH/2 - sy;
		if(sqrtf(dx*dx + dy*dy) > 260.0f && !tileMap->solidAt(planetX + planetW/2.0f, planetY + planetH/2.0f))
			break;
	}

	// A partir dos 45 s, 15% dos spawns viram minas de proximidade
	// (limite de 4 ativas): area negada com espoleta e reacao em cadeia.
	if(gameTimeMs > 45000 && mineArray.size() < 4 && (rand() % 100 < 15))
	{
		spawnMineAt(planetX, planetY);
		return;
	}

	PlanetRed *novo = new PlanetRed(sprite, planetX, planetY, roxo ? 4 : 2,
	                                (float)worldWidth, (float)worldHeight,
	                                roxo ? 25 : 10, roxo ? 60 : 20,
	                                roxo ? 0.55f : 1.0f, tileMap,
	                                roxo ? 8 : 5);
	planetArray.push_back(novo);
}

// Sorteio de mina dentro do spawn de planetas: a partir dos 45 s, 15%
// dos spawns viram minas de proximidade (limite de 4 ativas). A posicao
// ja foi validada acima (longe da nave, fora de painel solido).
void StateGame::spawnMineAt(int px, int py)
{
	int mineW = mineSprite->getRenderWidth();
	int mineH = mineSprite->getRenderHeight();
	int mx = clampi(px, 0, worldWidth - mineW);
	int my = clampi(py, 0, worldHeight - mineH);
	mineArray.push_back(new Mine(mineSprite, (float)mx, (float)my,
		(float)worldWidth, (float)worldHeight, tileMap));
}

void StateGame::detonateMine(std::vector<Mine*>::iterator vitima)
{
	Mine *m = *vitima;
	float ux = m->x + m->w/2;
	float uy = m->y + m->h/2;

	boomX = ux;
	boomY = uy;
	boomActive = true;
	boom->restart();
	sfx->play(0);
	if(shakeTime < 250)
		shakeTime = 250;

	destroyedCount++;
	addKillScore(15, ux, uy);
	spawnDrop(ux, uy, 25);

	// Reacao em cadeia: vizinhas a 140 px armam com 150 ms.
	for(std::vector<Mine*>::iterator o = mineArray.begin(); o != mineArray.end(); ++o)
	{
		if(o == vitima || (*o)->fuse == 0)
			continue;
		float odx = (*o)->x + (*o)->w/2 - ux;
		float ody = (*o)->y + (*o)->h/2 - uy;
		if(odx*odx + ody*ody < 140.0f * 140.0f)
		{
			if((*o)->fuse < 0 || (*o)->fuse > 150)
				(*o)->fuse = 150;
		}
	}

	// Dano em area (raio 130) com as mesmas regras dos planetas.
	if(ship)
	{
		float sdx = ship->x + ship->w/2 - ux;
		float sdy = ship->y + ship->h/2 - uy;
		if(sdx*sdx + sdy*sdy < 130.0f * 130.0f &&
		   invulnTime <= 0 && ship->getShieldTime() <= 0)
		{
			int dano = (int)(m->dano * GameSettings::damageMult());
			ship->hp -= dano;
			hud->setLife(ship->hp);
			invulnTime = 800;
			shakeTime = 250;
			tookDamage = true;

			char buf[16];
			snprintf(buf, sizeof(buf), "-%d", dano);
			addFloater(buf, ship->x + ship->w/2, ship->y + ship->h/2, makeColor(255, 90, 90), false, 900);
		}
	}

	delete(m);
	mineArray.erase(vitima);
}

int StateGame::nextPlanetInterval()
{
	// O intervalo base cai 20 ms por segundo de partida: ~4,2 s no inicio,
	// ~3,0 s em 1 min, ~1,8 s em 2 min e no minimo 1,3 s (por volta de
	// 2 min 25 s). A variacao de +-20% evita um ritmo metronomico. O
	// multiplicador da dificuldade ajusta o ritmo geral.
	int base = 4200 - 20 * (gameTimeMs / 1000);
	if(base < 1300)
		base = 1300;

	int variacao = base / 5;
	int intervalo = base - variacao + (rand() % (2 * variacao + 1));

	return (int)(intervalo * GameSettings::planetSpawnMult());
}

void StateGame::spawnPowerUp()
{
	// Maximo de 2 intens "de agenda" na tela ao mesmo tempo (drops a mais
	// sao tolerados ate 4 pelo limite interno do spawnDrop).
	if(powerUpArray.size() >= 2)
		return;

	// Proximo power-up nasce entre 15 e 25 segundos.
	powerUpCooldown = 15000 + (rand() % 10000);

	float sx = ship->x + ship->w/2;
	float sy = ship->y + ship->h/2;

	int kind = rand() % 4;
	int powerW = powerUpSprites[kind]->getRenderWidth();
	int powerH = powerUpSprites[kind]->getRenderHeight();

	// Mesma regra dos planetas: nasce na tela e dentro do mapa.
	int areaMinX = (int)Camera::getInstance().getX();
	int areaMaxX = areaMinX + SCREEN_W - powerW;
	int areaMinY = (int)Camera::getInstance().getY();
	int areaMaxY = areaMinY + SCREEN_H - powerH;
	if(areaMinX < 0)
		areaMinX = 0;
	if(areaMinY < 0)
		areaMinY = 0;
	if(areaMaxX > worldWidth - powerW)
		areaMaxX = worldWidth - powerW;
	if(areaMaxY > worldHeight - powerH)
		areaMaxY = worldHeight - powerH;
	if(areaMaxX < areaMinX || areaMaxY < areaMinY)
	{
		areaMinX = 0;
		areaMaxX = worldWidth - powerW;
		areaMinY = 0;
		areaMaxY = worldHeight - powerH;
	}

	int px = areaMinX + rand() % (areaMaxX - areaMinX + 1);
	int py = areaMinY + rand() % (areaMaxY - areaMinY + 1);
	for(int tentativa = 0; tentativa < 20; ++tentativa)
	{
		px = areaMinX + rand() % (areaMaxX - areaMinX + 1);
		py = areaMinY + rand() % (areaMaxY - areaMinY + 1);

		float dx = px - sx;
		float dy = py - sy;
		if(sqrtf(dx*dx + dy*dy) > 300.0f)
			break;
	}

	powerUpArray.push_back(new PowerUp(powerUpSprites[kind], px, py, kind));
}

void StateGame::endGame(bool win)
{
	// Pode ser chamado por mais de uma colisao no mesmo frame. So encerra
	// uma vez.
	if(returnState == 3)
		return;

	if(win)
	{
		if(GameSettings::gameMode() == MODE_PTS)
			tryAch(ACH_SURV);
		else
			tryAch(ACH_EARTH);
		if(!tookDamage)
			tryAch(ACH_NODMG);
	}

	// Estatisticas da partida (consumidas pela tela final e persistidas).
	int scoreFinal = score + (win ? 500 : 0);

	PlayStats::lastTimeMs() = gameTimeMs;
	PlayStats::lastDestroyed() = destroyedCount;
	PlayStats::lastScore() = scoreFinal;
	PlayStats::lastWave() = wave;
	PlayStats::lastCombo() = bestStreak;
	PlayStats::saveRun(scoreFinal, destroyedCount, ufosKilledThisRun);

	ship->stop();
	sfxEngine->setVolume(0);

	// "Solta" a camara (ficara livre na tela final).
	Camera::getInstance().unBind();

	// A explosao final (vitoria ou derrota) estoura na nave: o chefe pode
	// estar em qualquer lugar do mapa (ou no meio do respawn de onda).
	boomX = ship->x + ship->w/2;
	boomY = ship->y + ship->h/2;

	if(ufo)
	{
		delete(ufo);
		ufo = NULL;
	}

	boomActive = true;
	boom->restart();
	sfx->play(0);

	if(win)
		returnStack = 1;
	else
	{
		delete(ship);
		ship = NULL;
		returnStack = 0;
	}

	// Tela de vitoria/derrota.
	returnState = 3;
}

int StateGame::update(int dt)
{
	InputManager &im = InputManager::getInstance();

	// H mostra/esconde as dicas do rodape (que somem sozinhas aos 12 s).
	if(im.isKeyDown(SDLK_h) && hud)
		hud->toggleHints();

	// F alterna o flash reduzido (sem piscadas de alerta).
	if(im.isKeyDown(SDLK_f))
	{
		GameSettings::reduceFlash() = !GameSettings::reduceFlash();
		toast(GameSettings::reduceFlash() ? "FLASH REDUZIDO: ON" : "FLASH REDUZIDO: OFF");
	}

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

	// Tutorial: registra o que o jogador ja fez e mostra so o que falta.
	if(ship)
	{
		if(!tutThrustDone && ship->isAccelerating())
			tutThrustDone = true;
		if(!tutTurnDone && (ship->getRotation() > 8.0f || ship->getRotation() < -8.0f))
			tutTurnDone = true;
	}
	updateTutorial();

	// Modo pontos: sobreviver 3:00 vence (sem Terra no mapa).
	if(GameSettings::gameMode() == MODE_PTS && returnState != 3)
	{
		if(!survWarnShown && gameTimeMs >= 150000)
		{
			survWarnShown = true;
			toast("FALTAM 30 S!");
		}
		if(gameTimeMs >= 180000)
		{
			endGame(true);
			return returnState;
		}
	}

	// Planetas (deriva lenta; quicam no mapa dentro do proprio update).
	for(std::vector<Planet*>::iterator it = planetArray.begin(); it != planetArray.end(); ++it)
		(*it)->update(dt);

	// Minas (mesma deriva/quique; a espoleta conta no proprio update).
	for(std::vector<Mine*>::iterator it = mineArray.begin(); it != mineArray.end(); ++it)
		(*it)->update(dt);

	// Terra, Lua (inexistentes no modo pontos)
	if(earth)
		earth->update(dt);
	if(moon)
		moon->update(dt);

	// Chefe: perseguidor com fases, tiros, invocacao e troca de ondas.
	updateUfoBehavior(dt);

	// Torres de vigia.
	updateTurrets(dt);

	// Nave (guarda a posicao anterior para o quique nos tiles solidos).
	if(ship)
	{
		shipPrevX = ship->x;
		shipPrevY = ship->y;
		ship->update(dt);
	}

	// Gravidade da Terra: dentro do raio de influencia, a nave recebe uma
	// "corrente" que a puxa para o nucleo, com damping independente
	// do framerate para nao disparar sem controle (so no modo travessia).
	if(ship && earth)
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

		// Colisao com os paineis solidos do mapa.
		resolveShipTiles();
	}

	// Tiro: ESPACO ou botao 1 do mouse disparam continuamente (cadencia de
	// 220 ms) na direcao do bico da nave.
	if(shootCooldown > 0)
		shootCooldown -= dt;

	bool atirando = im.isKeyPressed(SDLK_SPACE) || im.isMouseHeld(1) || im.isMouseDown(1) || im.isJoyHeld(0);
	if(ship && atirando && shootCooldown <= 0)
	{
		shoot();
		shootCooldown = 220;
	}
	updateBullets(dt);

	// Combustivel e pontuacao por sobrevivencia.
	updateFuel(dt);
	updateSurvivalScore(dt);

	// Janela de combo.
	if(comboTimer > 0)
	{
		comboTimer -= dt;
		if(comboTimer <= 0)
		{
			comboStreak = 0;
			comboMult = 1.0f;
		}
	}

	// Timers de efeito
	if(invulnTime > 0)
		invulnTime -= dt;
	if(shakeTime > 0)
		shakeTime -= dt;

	// Planetas: surgem sozinhos. Quando a tela esta cheia, tenta de
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

	// Partida encerrada (morte/vitoria): atualiza o HUD e sai.
	if(returnState == 3)
	{
		hud->setBossLife(0, 0, wave);
		return returnState;
	}

	// Rastro do motor: volume proporcional a aceleracao (0 quando parada).
	if(ship && ship->isAccelerating())
		sfxEngine->setVolume(90);
	else
		sfxEngine->setVolume(0);

	// Textos flutuantes e conquistas continuas.
	updateFloaters(dt);
	checkAchievements();

	// HUD (estatisticas + barra de vida do chefe)
	if(ufo)
		hud->setBossLife(ufo->hp, ufo->maxHp, wave);
	else
		hud->setBossLife(0, 0, wave);
	hud->update(gameTimeMs, destroyedCount, score, comboMult, comboStreak);

	#ifdef __EMSCRIPTEN__
	// JSON de diagnostico para os testes automatizados (apenas no build web):
	// posicoes, vida do chefe, torres, tiros inimigos e estado da partida.
	debugMs += dt;
	if(debugMs >= 200)
	{
		debugMs = 0;
		char buf[320];
		snprintf(buf, sizeof(buf),
			"{\"t\":%d,\"st\":%d,\"p\":%d,\"x\":%.0f,\"y\":%.0f,\"r\":%.1f,"
			"\"uh\":%d,\"um\":%d,\"ux\":%.0f,\"uy\":%.0f,\"w\":%d,\"k\":%d,"
			"\"tp\":%d,\"tt\":%d,\"tm\":%d,\"es\":%d,\"f\":%.0f,\"sc\":%d,\"cb\":%d}",
			gameTimeMs, returnState, ship ? ship->hp : -1,
			(double)(ship ? ship->x : -1.0f), (double)(ship ? ship->y : -1.0f),
			(double)(ship ? ship->getRotation() : 0.0f),
			ufo ? ufo->hp : -1, ufo ? ufo->maxHp : 0,
			(double)(ufo ? ufo->x + ufo->w/2 : -1.0f), (double)(ufo ? ufo->y + ufo->h/2 : -1.0f),
			wave, ufosKilledThisRun, (int)planetArray.size(), (int)turretArray.size(),
			(int)mineArray.size(), enemyShotsFired, (double)fuel, score, comboStreak);

		EM_ASM({ window.__lokuras_debug = UTF8ToString($0); }, buf);

		// Hook de teste: window.__lokuras_killBoss = 1 invoca killUfo() de
		// verdade (para os smoke tests validarem a troca de onda sem precisar
		// de 20+ acertos mirando). Sem efeito no jogo normal.
		if(EM_ASM_INT({ var v = window.__lokuras_killBoss | 0; if(v) window.__lokuras_killBoss = 0; return v; }))
		{
			killUfo();
		}
	}
	#endif

	return returnState;
}

void StateGame::renderObjective(float cameraX, float cameraY)
{
	// Sem Terra (modo pontos), nao ha para onde apontar.
	if(!earth)
		return;

	// (inalterado: a seta amarela continua apontando para a Terra)
	float ecx = earth->x + earth->w/2 - cameraX;
	float ecy = earth->y + earth->h/2 - cameraY;

	if(ecx >= 0 && ecx <= SCREEN_W && ecy >= 0 && ecy <= SCREEN_H)
		return;

	float dx = ecx - SCREEN_W / 2.0f;
	float dy = ecy - SCREEN_H / 2.0f;
	float d = sqrtf(dx*dx + dy*dy);
	if(d < 1.0f)
		return;

	dx /= d;
	dy /= d;

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

void StateGame::renderThreat(float cameraX, float cameraY)
{
	// Seta vermelha para o UFO fora da tela (mesma conta da seta da Terra):
	// morte sem telegrafar a direcao e frustracao gratuita.
	if(!ufo || !ship)
		return;

	float ucx = ufo->x + ufo->w/2 - cameraX;
	float ucy = ufo->y + ufo->h/2 - cameraY;

	if(ucx >= 0 && ucx <= SCREEN_W && ucy >= 0 && ucy <= SCREEN_H)
		return;

	float dx = ucx - SCREEN_W / 2.0f;
	float dy = ucy - SCREEN_H / 2.0f;
	float d = sqrtf(dx*dx + dy*dy);
	if(d < 1.0f)
		return;

	dx /= d;
	dy /= d;

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

	float px = -dy;
	float py = dx;
	float hx = ax + dx * 30;
	float hy = ay + dy * 30;
	int cor = 0xFF4040;

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

	// Planetas (vermelhos e roxos)
	for(std::vector<Planet*>::iterator it = planetArray.begin(); it != planetArray.end(); ++it)
		(*it)->render(cameraX, cameraY);

	// Minas (piscam quando armadas; fixas com flash reduzido)
	for(std::vector<Mine*>::iterator it = mineArray.begin(); it != mineArray.end(); ++it)
	{
		if((*it)->fuse < 0 || GameSettings::reduceFlash() || (gameTimeMs / 120) % 2 == 0)
			(*it)->render(cameraX, cameraY);
	}

	// Terra (so no modo travessia)
	if(earth)
		earth->render(cameraX, cameraY);

	// Lua (so no modo travessia)
	if(moon)
		moon->render(cameraX, cameraY);

	// Torres
	for(std::vector<Turret*>::iterator it = turretArray.begin(); it != turretArray.end(); ++it)
		(*it)->render(cameraX, cameraY);

	// Power-ups, cada tipo com seu marcador branco acima do cristal
	// (forma alem da cor: cruz = vida, anel = escudo, seta = turbo,
	// losango = combustivel). Legenda no menu.
	for(std::vector<PowerUp*>::iterator it = powerUpArray.begin(); it != powerUpArray.end(); ++it)
	{
		(*it)->render(cameraX, cameraY);

		float pcx = (*it)->x + (*it)->w/2 - cameraX;
		float ptop = (*it)->y - cameraY - 12.0f;
		if((*it)->kind == PowerUp::VIDA) // cruz
		{
			SDLBase::drawLine(pcx - 6, ptop, pcx + 6, ptop, 0xFFFFFF, 1);
			SDLBase::drawLine(pcx, ptop - 6, pcx, ptop + 6, 0xFFFFFF, 1);
		}
		else if((*it)->kind == PowerUp::ESCUDO) // anel
			SDLBase::drawCircle((int)pcx, (int)ptop, 6, 0xFFFFFF);
		else if((*it)->kind == PowerUp::TURBO) // seta (triangulo)
		{
			SDLBase::drawLine(pcx - 6, ptop + 5, pcx, ptop - 6, 0xFFFFFF, 1);
			SDLBase::drawLine(pcx, ptop - 6, pcx + 6, ptop + 5, 0xFFFFFF, 1);
			SDLBase::drawLine(pcx - 6, ptop + 5, pcx + 6, ptop + 5, 0xFFFFFF, 1);
		}
		else // FUEL: losango
		{
			SDLBase::drawLine(pcx, ptop - 7, pcx + 6, ptop, 0xFFFFFF, 1);
			SDLBase::drawLine(pcx + 6, ptop, pcx, ptop + 7, 0xFFFFFF, 1);
			SDLBase::drawLine(pcx, ptop + 7, pcx - 6, ptop, 0xFFFFFF, 1);
			SDLBase::drawLine(pcx - 6, ptop, pcx, ptop - 7, 0xFFFFFF, 1);
		}
	}

	// UFO (sem linhas de fila no modo perseguidor)
	if(ufo)
	{
		if(!ufo->isAutonomous())
			ufo->renderQueueLines(cameraX, cameraY);
		ufo->render(cameraX, cameraY);
	}

	// Tiros: os da nave tem rastro azul; os inimigos, rastro alaranjado
	// (facil de distinguir no susto).
	for(std::vector<Bullet>::iterator it = bulletArray.begin(); it != bulletArray.end(); ++it)
	{
		int bx = (int)(it->x - cameraX);
		int by = (int)(it->y - cameraY);
		int tx = (int)(it->x - it->vx * 18 - cameraX);
		int ty = (int)(it->y - it->vy * 18 - cameraY);

		if(it->enemy)
		{
			SDLBase::drawLine(tx, ty, bx, by, 0xC84A12, 1);
			SDLBase::drawRect(bx - 3, by - 3, 6, 6, 0xFF8A3D);
			SDLBase::drawRect(bx - 1, by - 1, 2, 2, 0xFFFFFF);
		}
		else
		{
			SDLBase::drawLine(tx, ty, bx, by, 0x3FA8FF, 1);
			SDLBase::drawRect(bx - 3, by - 3, 6, 6, 0x7DE1FF);
			SDLBase::drawRect(bx - 1, by - 1, 2, 2, 0xFFFFFF);
		}
	}

	// Nave (pisca durante os i-frames, exceto com flash reduzido).
	if(ship)
	{
		if(invulnTime <= 0 || GameSettings::reduceFlash() || (gameTimeMs / 90) % 2 == 0)
			ship->render(cameraX, cameraY);

		if(ship->getShieldTime() > 0)
		{
			float scx = ship->x + ship->w/2 - cameraX;
			float scy = ship->y + ship->h/2 - cameraY;
			SDLBase::drawCircle((int)scx, (int)scy, 64, 0x00E5C8);
		}

		// Guia de mira: linha curta no bico da nave (o tiro sai dali).
		float angulo = (ship->getRotation() - 90.0f) * PI / 180.0f;
		float dirX = -cosf(angulo);
		float dirY = sinf(angulo);
		float scx = ship->x + ship->w/2 - cameraX;
		float scy = ship->y + ship->h/2 - cameraY;
		SDLBase::drawLine(scx + dirX * 58.0f, scy + dirY * 58.0f,
		                  scx + dirX * 130.0f, scy + dirY * 130.0f, 0x80D0FF, 1);

		// Legenda visual de perigo proximo: "!" sobre a nave quando um
		// tiro inimigo esta a menos de 180 px (o som por si so nao diz
		// de onde vem).
		bool tiroPerto = false;
		for(std::vector<Bullet>::iterator b = bulletArray.begin(); b != bulletArray.end(); ++b)
		{
			if(!b->enemy)
				continue;
			float bdx = b->x - (ship->x + ship->w/2);
			float bdy = b->y - (ship->y + ship->h/2);
			if(bdx*bdx + bdy*bdy < 180.0f * 180.0f)
			{
				tiroPerto = true;
				break;
			}
		}
		if(tiroPerto)
		{
			SDLBase::drawRect((int)scx - 2, (int)scy - 92, 4, 16, 0xFFE14D);
			SDLBase::drawRect((int)scx - 2, (int)scy - 72, 4, 4, 0xFFE14D);
		}
	}

	// Explosao (uma vez, centrada no ponto da colisao)
	if(boomActive)
		boom->render(boomX - cameraX - boom->getClip()->w/2,
		             boomY - cameraY - boom->getClip()->h/2);

	// Seta de objetivo (Terra fora da tela) e de ameaca (UFO fora da tela).
	renderObjective(cameraX, cameraY);
	renderThreat(cameraX, cameraY);

	// Textos flutuantes (pontos/dano/avisos) por cima do mundo, abaixo do HUD.
	renderFloaters(cameraX, cameraY);

	// HUD
	hud->render();

	// Sombra da pausa
	if(paused)
		hud->renderPause();
}
