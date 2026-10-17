/*
 * TileMap.cpp
 *
 *  Created on: 13/05/2013
 *      Author: Victor
 */

#include "TileMap.h"

TileMap::TileMap(int mapWidth, int mapHeight, int mapLayers, TileSet* tileSet)
{
	// Inicializa as variáveis do TileMap: width, height, layers e tileset.

	this->mapWidth = mapWidth;
	this->mapHeight = mapHeight;
	this->mapLayers = mapLayers;
	this->tileSet = tileSet;

	// Redimensiona a tileMatrix para as dimensões corretas e a inicializa com 0.

	tileMatrix.resize(mapWidth);

	unsigned int x = 0;
	unsigned int y = 0;
	unsigned int z = 0;

	for (x = 0; x < tileMatrix.size(); ++x)
		tileMatrix[x].resize(mapHeight);

	for (x = 0; x < tileMatrix.size(); ++x)
		for (y = 0; y < tileMatrix[0].size(); ++y)
			tileMatrix[x][y].resize(mapLayers);

	// Preenche a matriz com 0

	for (x = 0; x < tileMatrix.size(); ++x)
		for (y = 0; y < tileMatrix[x].size(); ++y)
			for (z = 0; z < tileMatrix[x][y].size(); ++z)
				tileMatrix[x][y][z] = 0;
}

TileMap::TileMap(std::string mapa, TileSet* tileSet)
{
	// Define o tileset e inicializa width, height e layers com 0.

	this->tileSet = tileSet;
	mapWidth = 0;
	mapHeight = 0;
	mapLayers = 0;

	// Chama o método load(mapa) para carregar o tileMap do arquivo.

	load(mapa);
}

void TileMap::load(std::string mapPath)
{
	// Abre o arquivo informado no PARAMETRO (antes o nome da linha abaixo era
	// fixo e o parametrto mapPath era simplesmente ignorado).
	ifstream map(mapPath.c_str());
	string value;

	if(!map.is_open())
	{
		cerr << "TileMap::load: nao foi possivel abrir o mapa '" << mapPath << "'" << endl;
		return;
	}

	getline( map, value, ',' );
	mapWidth = atoi(value.c_str());
	getline( map, value, ',' );
	mapHeight = atoi(value.c_str());
	getline( map, value, ',' );
	mapLayers = atoi(value.c_str());

	tileMatrix.resize(mapWidth);

	for (unsigned int x = 0; x < tileMatrix.size(); ++x)
		tileMatrix[x].resize(mapHeight);

	for (unsigned int x = 0; x < tileMatrix.size(); ++x)
		for (unsigned int y = 0; y < tileMatrix[0].size(); ++y)
			tileMatrix[x][y].resize(mapLayers);

	// Preenche a matriz com 0
	for (unsigned int x = 0; x < tileMatrix.size(); ++x)
		for (unsigned int y = 0; y < tileMatrix[x].size(); ++y)
			for (unsigned int z = 0; z < tileMatrix[x][y].size(); ++z)
				tileMatrix[x][y][z] = 0;

	for(int i = 0; i < mapLayers; ++i)
		for(int j = 0; j <  mapWidth; ++j)
			for(int k = 0; k < mapHeight; ++k)
			{
				getline( map, value, ',' );
				tileMatrix[j][k][i] = atoi(value.c_str());
			}
	map.close();
}

void TileMap::setTileSet (TileSet* tileSet)
{
	// Redefine o tileset usado na renderização.

	this->tileSet = tileSet;
}

int& TileMap::at(int x, int y, int z)
{
	// Retorna uma referência para o conteúdo do tileMap na posição x,y,z.
	// Para facilitar o acesso a matrizes 2D, é definido 0 como padrão em z.
	// Deve funcionar como um acessor, igual ao da classe vector.

	return tileMatrix[x][y][z];
}

void TileMap::render(float cameraX, float cameraY)
{
	// Checa se o tileSet não é nulo.

	if (tileSet != NULL)
	{
		// Itera em toda a matriz, checando se o índice armazenado é > 0. Caso seja,
		// renderiza aquele tile.

		int index = 0;
		float posX = 0;
		float posY = 0;
		int tileWidth = tileSet->getTileWidth();
		int tileHeight = tileSet->getTileHeight();

		// Número para o cálculo do parallax
		float parallaxConst = 0.75 / mapLayers;

		for (int i = 0; i < mapLayers; i++)
			for (int j = 0; j < mapWidth; j++)
				for (int k = 0; k < mapHeight; k++)
				{
					// Mesma ordem de indices do load([x][y][z]): o render antigo
					// lia [k][j] e so "funcionava" porque o mapa era quadrado.
					index = tileMatrix[j][k][i];
					if (index > 0)
					{
						// A constante que multiplica a câmera nunca é maior que 1
						// Isso garante que os objetos "soltos" no mapa se movam na
						// mesma velocidade da camada superior do tilemap.
						posX = j * tileWidth - cameraX * (0.25 + parallaxConst * (i + 1));
						posY = k * tileHeight - cameraY * (0.25 + parallaxConst * (i + 1));

						// Culling: tile inteiramente fora da tela nao vai para a GPU.
						if(posX > SCREEN_W || posY > SCREEN_H ||
						   posX < -tileWidth || posY < -tileHeight)
							continue;

						tileSet->render(index - 1, posX, posY);
					}
				}
	}
}

int TileMap::width ()
{
	return mapWidth;
}

int TileMap::height ()
{
	return mapHeight;
}

int TileMap::layers ()
{
	return mapLayers;
}
