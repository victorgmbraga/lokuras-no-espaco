/*
 * PlanetRed.h
 *
 *  Created on: 27/05/2013
 *      Author: Victor
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
		float vx, vy;     // deriva lenta (px/ms)
		float mundoW, mundoH; // limites do mapa atual (px)
	public:
		PlanetRed(Sprite* sprite, float x, float y, int hp = 1,
		          float mundoW = WORLD_W, float mundoH = WORLD_H);
		virtual void update(int dt);
};


#endif /* PLANETRED_H_ */
