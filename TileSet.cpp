/*
 * TileSet.cpp
 *
 *  Created on: 13/05/2013
 *      Author: Victor
 */

#include "TileSet.h"

TileSet::TileSet (int tileWidth, int tileHeight, std::string filePath)
{
	this->tileWidth = tileWidth;
	this->tileHeight = tileHeight;
	//vTiles = NULL;
	tileSet = new Sprite(filePath);
	lines = (tileSet->getWidth())/tileWidth;
	columns = (tileSet->getHeight())/tileHeight;
	destRect = NULL;
}

TileSet::TileSet (std::string filePath, int lines, int columns)
{
	this->lines = lines;
	this->columns = columns;
	//vTiles = NULL;
	tileSet = new Sprite(filePath);
	tileWidth = (tileSet->getWidth())/lines;
	tileHeight = (tileSet->getHeight())/columns;
	destRect = NULL;
}

TileSet::TileSet (int tileWidth, int tileHeight)
{
	lines = 0;
	columns = 0;
	this->tileWidth = tileWidth;
	this->tileHeight = tileHeight;
	tileSet = NULL;
	//vTiles = new std::vector<Sprite*>;
	destRect = NULL;
}

void TileSet::addTile (string filePath)
{
	if(!usingSingleFile())
	{
		Sprite *tile = new Sprite(filePath);
		vTiles.push_back(tile);
	}
}

void TileSet::render (int index, float posX, float posY)
{
	if (usingSingleFile()) // Falta implementar
	{
		//int numTiles = lines*columns;
		int indexLine = (index / columns) + 1;
		int indexColumn = index % columns;
	}
	else
	{
		vTiles.at(index)->render(posX, posY);
	}
}

bool TileSet::usingSingleFile (void)
{
	if (tileSet != NULL)
		return true;
	else
		return false;
}

int TileSet::getTileWidth (void)
{
	return tileWidth;
}

int TileSet::getTileHeight (void)
{
	return tileHeight;
}
