/*
 * TileMap.h
 *
 *  Created on: 13/05/2013
 *      Author: Victor
 */

#ifndef TILEMAP_H_
#define TILEMAP_H_

// Dimensoes do mundo: mapa de TILEMAP_SIZE^2 tiles de TILE_SIZE px
// (16 x 16 tiles de 75 px = mundo de 1200 x 1200 px).
#define TILEMAP_SIZE 16
#define TILE_SIZE 75

#include "TileSet.h"
#include <stdlib.h>
#include <iostream>
#include <fstream>
using namespace std;

class TileMap
{
	private:
		std::vector<std::vector <std::vector <int> > > tileMatrix;
		TileSet *tileSet;
		int mapWidth, mapHeight, mapLayers;

	public:
		TileMap(int mapWidth, int mapHeight, int mapLayers = 1, TileSet* tileSet = NULL);
		TileMap(std::string mapa, TileSet* tileSet = NULL);
		void load(std::string mapPath);
		void setTileSet(TileSet* tileSet);
		int& at(int x, int y, int z = 0);
		void render(float cameraX = 0.0, float cameraY = 0.0);
		int width();
		int height();
		int layers();
};


#endif /* TILEMAP_H_ */
