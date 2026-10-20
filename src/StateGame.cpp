/*
 * GameManager.cpp
 *
 *  Created on: 16/04/2013
 *      Author: Victor
 */

#include "StateGame.h"
#include <cmath>

// Dano do UFO: 30% da vida maxima da nave (100).
static const int UFO_COLLISION_DAMAGE = 30;

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

void StateGame::load(int stack)
{
	// Recorde persistido (arquivo lokuras.rec).
	PlayStats::load();

	// Background
	bg = new Sprite("img/fundo.png");
	bg->resizeCover(SCREEN_W, SCREEN_H);

	// TileSet e mapa aleatorio. O tamanho maximo e TILEMAP_MAX_SIZE x
	// TILEMAP_MAX_SIZE tiles; cada partida sorteia um mundo dentro do limite.
	tileSet = new TileSet(TILE_SIZE, TILE_SIZE, "img/Tileset.png");
	buildRandomMap();

	// Nave (criada antes da Terra e do UFO, que precisam da posicao dela)
	shipAnimation = new Animation("img/NaveSheet.png", 200, 4);
	ship = new AccObject(shipAnimation, 0, 0, 94, 100, 100);

	// Terra (escala assada na surface: hitbox == imagem renderizada)
	earthSprite = new Sprite("img/PlanetaTerra.png");
	earthSprite->scale(3);
	earth = new Earth(earthSprite, 0, 0, 100);

	// UFO perseguidor autonomo
	ufoSprite = new Sprite("img/ufo.png");
	ufoSprite->scale(3);
	ufo = new FollowerObject(ufoSprite, 0, 0);
	// O PNG 32x32 escalado 3x tem margens transparentes em cima/embaixo;
	// usa uma hitbox central menor, sem deslocar o desenho do UFO.
	ufo->setHitbox(80, 48);
	ufo->setAutonomous(true);
	ufo->setChaseTarget(ship);
	ufo->setChaseSpeed(0.22);

	// Terra e UFO surgem longe da nave e um do outro.
	placeEarthAndUfo();

	// Lua (orbita a Terra ja posicionada e machuca a nave)
	moonSprite = new Sprite("img/Lua.png");
	moonSprite->scale(3);
	moon = new Moon(moonSprite, 1, earth);

	// Planetas Vermelhos: a partida comeca com alguns ja espalhados pelo
	// mapa; depois eles surgem sozinhos com ritmo crescente.
	redPlanetSprite = new Sprite("img/PlanetaVermelho.png");
	spawnInitialPlanets();
	redPlanetCooldown = 2500 + (rand() % 1500);

	// Explosao (one-shot: toca uma vez e congela no ultimo quadro)
	boom = new Animation("img/BoomSheet.png", 200, 8);
	boom->setOneShot(true);
	boomX = 0;
	boomY = 0;
	boomActive = false;

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
	hud->setBossLife(ufo->hp, ufo->maxHp);

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

	// Variáveis de retorno
	returnState = 0;
	returnStack = 0;
}

void StateGame::buildRandomMap()
{
	// Dimensoes sorteadas dentro do limite (13 a 16 tiles de lado).
	int mapW = TILEMAP_MIN_SIZE + rand() % (TILEMAP_MAX_SIZE - TILEMAP_MIN_SIZE + 1);
	int mapH = TILEMAP_MIN_SIZE + rand() % (TILEMAP_MAX_SIZE - TILEMAP_MIN_SIZE + 1);

	worldWidth = mapW * TILE_SIZE;
	worldHeight = mapH * TILE_SIZE;

	tileMap = new TileMap(mapW, mapH, 2, tileSet);

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
}

void StateGame::placeEarthAndUfo()
{
	if(!ship || !earth || !ufo)
		return;

	float shipCX = ship->x + ship->w/2;
	float shipCY = ship->y + ship->h/2;

	// Distancias proporcionais ao tamanho do mundo: os dois objetos ficam
	// longe da nave e um do outro em qualquer um dos tamanhos sorteados.
	float diagonal = sqrtf((float)(worldWidth * worldWidth + worldHeight * worldHeight));
	float minPlayerDist = diagonal * 0.50f;
	float minEarthUfoDist = diagonal * 0.42f;

	// Terra: margem extra para a orbita da Lua (raio 200 + metade da Lua)
	// caber dentro do mapa.
	float earthHalfW = earth->w / 2;
	float earthHalfH = earth->h / 2;
	float earthMargin = earthHalfW + 250.0f;
	if(earthMargin > worldWidth / 2)
		earthMargin = worldWidth / 2;
	if(earthMargin > worldHeight / 2)
		earthMargin = worldHeight / 2;

	// Grade de posicoes validas para a Terra (longe da nave), sorteada no
	// final. A margem tambem mantem a orbita da Lua dentro do mapa.
	struct Ponto { float x, y; };
	std::vector<Ponto> candidatosTerra;

	float earthX = worldWidth / 2.0f;
	float earthY = worldHeight / 2.0f;
	float bestDist = -1.0f;
	const float passo = 20.0f;
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

	float earthCX = earth->x + earthHalfW;
	float earthCY = earth->y + earthHalfH;

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
			float score = dPlayer < dEarth ? dPlayer : dEarth;

			if(score > bestDist)
			{
				bestDist = score;
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

void StateGame::spawnInitialPlanets()
{
	if(!redPlanetSprite || !ship || !earth || !ufo)
		return;

	float planetW = redPlanetSprite->getRenderWidth();
	float planetH = redPlanetSprite->getRenderHeight();
	float shipCX = ship->x + ship->w/2;
	float shipCY = ship->y + ship->h/2;
	float earthCX = earth->x + earth->w/2;
	float earthCY = earth->y + earth->h/2;
	float ufoCX = ufo->x + ufo->w/2;
	float ufoCY = ufo->y + ufo->h/2;

	// Pequena quantidade inicial: 2 a 4 planetas ja posicionados no mapa.
	int quantidade = 2 + rand() % 3;

	for(int i = 0; i < quantidade; ++i)
	{
		// Tambem usa uma grade de posicoes validas: garante que os planetas
		// iniciais nao nascam em cima da nave, da Terra, do UFO ou uns dos
		// outros, e sorteia entre as opcoes.
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

				float score = dPlayer;
				if(dEarth < score) score = dEarth;
				if(dUfo < score) score = dUfo;
				if(dOther < score) score = dOther;

				if(score > bestScore)
				{
					bestScore = score;
					bestX = x;
					bestY = y;
				}

				if(dPlayer >= 280.0f && dEarth >= 220.0f &&
				   dUfo >= 170.0f && dOther >= 120.0f)
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

		planetArray.push_back(new PlanetRed(redPlanetSprite, bestX, bestY, 2,
		                                    (float)worldWidth, (float)worldHeight));
	}
}

int StateGame::unload()
{
	delete(bg);
	delete(tileMap);
	delete(tileSet);
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

	// UFO: encostar custa 30 de vida (30% do maximo), com os mesmos i-frames
	// dos planetas/lua. O escudo evita o dano enquanto estiver ativo.
	if(ufo && ship->collidesWith(ufo) && invulnTime <= 0 && ship->getShieldTime() <= 0)
	{
		boomX = ufo->x + ufo->w/2;
		boomY = ufo->y + ufo->h/2;
		boomActive = true;
		boom->restart();

		ship->hp -= UFO_COLLISION_DAMAGE;
		hud->setLife(ship->hp);
		sfx->play(0);
		invulnTime = 800;
		shakeTime = 250;
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

		// Planetas vermelhos: 2 tiros (1 de dano por projetil).
		if(!remove)
		{
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

						delete(*p);
						planetArray.erase(p);
					}

					remove = true;
					break;
				}
			}
		}

		// UFO: chefe com UFO_MAX_HP tiros. Derrota-lo vence a partida.
		if(!remove && ufo && bulletHit(*it, ufo))
		{
			ufo->hp -= 1;
			if(ufo->hp <= 0)
			{
				endGame(true);
				return;
			}

			remove = true;
		}

		if(remove)
			it = bulletArray.erase(it);
		else
			++it;
	}
}

void StateGame::addPlanet()
{
	if(ship == NULL)
		return;

	float sx = ship->x + ship->w/2;
	float sy = ship->y + ship->h/2;

	int planetW = redPlanetSprite->getRenderWidth();
	int planetH = redPlanetSprite->getRenderHeight();

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

	// Posicao aleatoria na tela, tentando nascer longe da nave.
	int planetX = areaMinX + rand() % (areaMaxX - areaMinX + 1);
	int planetY = areaMinY + rand() % (areaMaxY - areaMinY + 1);
	for(int tentativa = 0; tentativa < 20; ++tentativa)
	{
		planetX = areaMinX + rand() % (areaMaxX - areaMinX + 1);
		planetY = areaMinY + rand() % (areaMaxY - areaMinY + 1);

		float dx = planetX - sx;
		float dy = planetY - sy;
		if(sqrtf(dx*dx + dy*dy) > 260.0f)
			break;
	}

	// 2 tiros: cada projetil tira 1 de vida.
	PlanetRed *planetRed = new PlanetRed(redPlanetSprite, planetX, planetY, 2,
	                                     (float)worldWidth, (float)worldHeight);
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
	// Pode ser chamado por mais de uma colisao no mesmo frame (ex.: o tiro
	// que derrota o UFO e a nave encostando na Terra). So encerra uma vez.
	if(returnState == 3)
		return;

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

	// Tiro: ESPACO ou botao 1 do mouse disparam continuamente (cadencia de
	// 220 ms) na direcao do bico da nave.
	if(shootCooldown > 0)
		shootCooldown -= dt;

	bool atirando = im.isKeyPressed(SDLK_SPACE) || im.isMouseHeld(1) || im.isMouseDown(1);
	if(ship && atirando && shootCooldown <= 0)
	{
		shoot();
		shootCooldown = 220;
	}
	updateBullets(dt);

	// UFO derrotado: encerra antes das colisoes restantes deste frame.
	if(returnState == 3)
	{
		hud->setBossLife(0, 0);
		return returnState;
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

	// HUD (estatisticas + barra de vida do chefe)
	if(ufo)
		hud->setBossLife(ufo->hp, ufo->maxHp);
	else
		hud->setBossLife(0, 0);
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

	// Tiros da nave: rastro luminoso + nucleo branco.
	for(std::vector<Bullet>::iterator it = bulletArray.begin(); it != bulletArray.end(); ++it)
	{
		int bx = (int)(it->x - cameraX);
		int by = (int)(it->y - cameraY);
		int tx = (int)(it->x - it->vx * 18 - cameraX);
		int ty = (int)(it->y - it->vy * 18 - cameraY);

		SDLBase::drawLine(tx, ty, bx, by, 0x3FA8FF, 1);
		SDLBase::drawRect(bx - 3, by - 3, 6, 6, 0x7DE1FF);
		SDLBase::drawRect(bx - 1, by - 1, 2, 2, 0xFFFFFF);
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
