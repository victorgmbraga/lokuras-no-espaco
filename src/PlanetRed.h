/*
 * PlanetRed.h
 *
 *  Created on: 27/05/2013
 *      Author: Victor
 *
 *  Planeta perigoso da partida (vermelho ou roxo). Deriva devagar, quica nas
 *  bordas do mapa aleatorio e nos tiles solidos. Guarda o que rende quando
 *  destruido (pontos e chance de drop de power-up).
 */

#ifndef PLANETRED_H_
#define PLANETRED_H_

// Limites padrao usados quando nenhum tamanho de mundo e informado.
// O StateGame passa as dimensoes reais do mapa aleatorio da partida.
#define WORLD_W (TILEMAP_MAX_SIZE * TILE_SIZE)
#define WORLD_H (TILEMAP_MAX_SIZE * TILE_SIZE)

#include "TileMap.h" // TILEMAP_MAX_SIZE / TILE_SIZE
#include "Planet.h"

class PlanetRed: public Planet
{
	private:
		float vx, vy;          // deriva lenta (px/ms)
		float mundoW, mundoH;  // limites do mapa atual (px)
		TileMap *mapa;         // tiles solidos do mapa atual (NULL = sem tiles)
	public:
		int valor;             // pontos quando destruido
		int dropChance;        // % de chance de dropar power-up
		int dano;              // dano de colisão com a nave

		PlanetRed(Sprite* sprite, float x, float y, int hp = 1,
		          float mundoW = WORLD_W, float mundoH = WORLD_H,
		          int valor = 10, int dropChance = 20,
		          float deriva = 1.0f, TileMap* mapa = NULL, int dano = 5);
		virtual void update(int dt);
};


#endif /* PLANETRED_H_ */
