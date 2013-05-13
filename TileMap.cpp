/*
 * TileMap.cpp
 *
 *  Created on: 13/05/2013
 *      Author: Victor
 */

#include "TileMap.h"

TileMap::TileMap (int mapWidth, int mapHeight, int mapLayers, TileSet* tileSet)
{
	// Inicializa as variáveis do TileMap: width, height, layers e tileset.

	this->mapWidth = mapWidth;
	this->mapHeight = mapHeight;
	this->mapLayers = mapLayers;
	this->tileSet = tileSet;

	// Redimensiona a tileMatrix para as dimensões corretas e a inicializa com -1.

	tileMatrix.resize(mapWidth);

	unsigned int x = 0;
	unsigned int y = 0;
	unsigned int z = 0;

	for (x = 0; x < tileMatrix.size(); ++x)
		tileMatrix[x].resize(mapHeight);

	for (x = 0; x < tileMatrix.size(); ++x)
		for (y = 0; y < tileMatrix[0].size(); ++y)
			tileMatrix[x][y].resize(mapLayers);

	// Preenche a matriz com -1

	for (x = 0; x < tileMatrix.size(); ++x)
		for (y = 0; y < tileMatrix[x].size(); ++y)
			for (z = 0; z < tileMatrix[x][y].size(); ++z)
				tileMatrix[x][y][z] = -1;
}

TileMap::TileMap (std::string mapa, TileSet* tileSet)
{
	// Define o tileset e inicializa width, height e layers com 0.

	this->tileSet = tileSet;
	mapWidth = 0;
	mapHeight = 0;
	mapLayers = 0;

	// Chama o método load(mapa) para carregar o tileMap do arquivo.

	load(mapa); // Falta implementar
}

void TileMap::load (std::string mapPath)
{
	// Falta implementar
}

void TileMap::setTileSet (TileSet* tileSet)
{
	// Redefine o tileset usado na renderização.

	this->tileSet = tileSet;
}

int& TileMap::at (int x, int y, int z)
{
	// Retorna uma referência para o conteúdo do tileMap na posição x,y,z.
	// Para facilitar o acesso a matrizes 2D, é definido 0 como padrão em z.
	// Deve funcionar como um acessor, igual ao da classe vector.

	return tileMatrix[x][y][z];
}

void TileMap::render (float cameraX, float cameraY)
{
	// Checa se o tileSet não é nulo.

	if (tileSet != NULL)
	{
		// Itera em toda a matriz, checando se o índice armazenado é < 0. Caso seja,
		// renderiza aquele tile.

		int index = 0;
		int posX = 0;
		int posY = 0;
		int tileWidth = tileSet->getTileWidth();
		int tileHeight = tileSet->getTileHeight();

		for (int i = 0; i < mapLayers; i++)
			for (int j = 0; j < mapWidth; j++)
				for (int k = 0; k < mapHeight; k++)
				{
					index = tileMatrix[j][k][i];
					if (index > 0)
					{
						posX = j*tileWidth;
						posY = k*tileHeight;
						tileSet->render(index, posX, posY);
					}
				}

	}
}

int TileMap::width (void)
{
	return mapWidth;
}

int TileMap::height (void)
{
	return mapHeight;
}

int TileMap::layers (void)
{
	return mapLayers;
}
