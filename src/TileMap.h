/*
 * TileMap.h
 *
 *  Created on: 13/05/2013
 *      Author: Victor
 */

#ifndef TILEMAP_H_
#define TILEMAP_H_

// Dimensoes do mundo: cada partida gera um mapa aleatorio entre
// TILEMAP_MIN_SIZE e TILEMAP_MAX_SIZE tiles de lado (75 px cada). O limite
// maximo de 22 x 22 tiles resulta em um mundo de 1650 x 1650 px, maior que
// a tela de 1280 x 720: explorar e seguir a seta voltam a importar.
#define TILEMAP_MIN_SIZE 18
#define TILEMAP_MAX_SIZE 22
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
		// Colisao com o mapa: a camada 1 anda 1:1 com a camera (mesma "escala"
		// dos objetos do mundo), entao os tiles dela sao solidos. Fora do mapa
		// (ou com uma camada so) nao ha obstaculo.
		bool solidAt(float x, float y) const;
		void render(float cameraX = 0.0, float cameraY = 0.0);
		int width();
		int height();
		int layers();
};


#endif /* TILEMAP_H_ */
