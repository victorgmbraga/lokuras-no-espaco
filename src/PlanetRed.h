/*
 * PlanetRed.h
 *
 *  Created on: 27/05/2013
 *      Author: Victor
 */

#ifndef PLANETRED_H_
#define PLANETRED_H_

// Mundo jogavel = area do tilemap (TILEMAP_SIZE^2 tiles).
#define WORLD_W (TILEMAP_SIZE * TILE_SIZE)
#define WORLD_H (TILEMAP_SIZE * TILE_SIZE)

#include "TileMap.h" // TILEMAP_SIZE / TILE_SIZE
#include "Planet.h"

class PlanetRed: public Planet
{
	private:
		float vx, vy; // deriva lenta (px/ms)
	public:
		PlanetRed(Sprite* sprite, float x, float y, int hp = 1);
		virtual void update(int dt);
};


#endif /* PLANETRED_H_ */
