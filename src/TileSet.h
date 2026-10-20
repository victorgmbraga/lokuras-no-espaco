/*
 * TileSet.h
 *
 *  Created on: 13/05/2013
 *      Author: Victor
 */

#ifndef TILESET_H_
#define TILESET_H_

#include "Sprite.h"
#include <vector>

class TileSet
{
	private:
		int tileWidth, tileHeight;
		int columns; // tiles na horizontal
		int lines;   // tiles na vertical
		Sprite* tileSet;
		std::vector<Sprite*> vTiles;
	public:
		TileSet(int tileWidth, int tileHeight, std::string filePath);
		TileSet(std::string filePath, int lines, int columns);
		TileSet(int tileWidth, int tileHeight);
		virtual ~TileSet();
		void addTile(string filePath);
		void render(int index, float posX, float posY);
		bool usingSingleFile();
		int getTileWidth();
		int getTileHeight();
		// Quantidade total de tiles da imagem (usada pelo mapa aleatorio).
		int getTileCount();
};


#endif /* TILESET_H_ */
