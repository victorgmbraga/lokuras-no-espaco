/*
 * PlanetRed.cpp
 *
 *  Created on: 27/05/2013
 *      Author: Victor
 */

#include "PlanetRed.h"

PlanetRed::PlanetRed(Sprite* sprite, float x, float y, int hp,
                     float mundoW, float mundoH, int valor, int dropChance,
                     float deriva, TileMap* mapa, int dano): Planet(sprite, x, y, hp)
{
	// Deriva lenta e aleatoria. srand() precisa ter sido semeado.
	// "deriva" escala a velocidade (planeta roxo e mais pesado/lento).
	vx = ((rand() % 200) - 100) * 0.0002f * deriva;    // ate +-0.02 px/ms
	vy = ((rand() % 200) - 100) * 0.0002f * deriva;
	this->mundoW = mundoW;
	this->mundoH = mundoH;
	this->mapa = mapa;
	this->valor = valor;
	this->dropChance = dropChance;
	this->dano = dano;
}

void PlanetRed::update(int dt)
{
	float oldX = x, oldY = y;

	x += vx * dt;
	y += vy * dt;

	// Quica nas bordas do mundo gerado para esta partida.
	if(x < 0)
	{
		x = 0;
		vx = -vx;
	}
	if(y < 0)
	{
		y = 0;
		vy = -vy;
	}
	if(x + w > mundoW)
	{
		x = mundoW - w;
		vx = -vx;
	}
	if(y + h > mundoH)
	{
		y = mundoH - h;
		vy = -vy;
	}

	// Quica nos tiles solidos (camada 1 do mapa): amostra as bordas do
	// planeta e, se algum ponto encostou em painel, reverte e inverte o eixo.
	if(mapa)
	{
		float midX = x + w / 2;
		float midY = y + h / 2;

		bool cantoEsqTopo = mapa->solidAt(x, y);
		bool cantoDirTopo = mapa->solidAt(x + w, y);
		bool cantoEsqFundo = mapa->solidAt(x, y + h);
		bool cantoDirFundo = mapa->solidAt(x + w, y + h);

		bool bateEsquerda = cantoEsqTopo || cantoEsqFundo || mapa->solidAt(x, midY);
		bool bateDireita  = cantoDirTopo || cantoDirFundo || mapa->solidAt(x + w, midY);
		bool bateCima     = cantoEsqTopo || cantoDirTopo || mapa->solidAt(midX, y);
		bool bateBaixo    = cantoEsqFundo || cantoDirFundo || mapa->solidAt(midX, y + h);

		if(bateEsquerda || bateDireita)
		{
			x = oldX;
			vx = -vx;
		}
		if(bateCima || bateBaixo)
		{
			y = oldY;
			vy = -vy;
		}
	}
}
