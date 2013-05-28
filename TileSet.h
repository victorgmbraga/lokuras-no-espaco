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
		int lines, columns;
		Sprite* tileSet;
		std::vector<Sprite*> vTiles;
		SDL_Rect* destRect;
	public:
		TileSet(int tileWidth, int tileHeight, std::string filePath);
		TileSet(std::string filePath, int lines, int columns);
		TileSet(int tileWidth, int tileHeight);
		void addTile(string filePath);
		void render(int index, float posX, float posY);
		bool usingSingleFile();
		int getTileWidth();
		int getTileHeight();
};


#endif /* TILESET_H_ */
