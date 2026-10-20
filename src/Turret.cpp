/*
 * Turret.cpp
 *
 *  Created on: 20/10/2026
 *      Author: Victor
 */

#include "Turret.h"
#include <cmath>

// AccObject.h define PI, mas a Turret nao depende dele: constante propria.
static const float TURRET_PI = 3.14159265f;

Turret::Turret(Sprite* sprite, float x, float y):
	GameObject(x, y, sprite->getRenderWidth(), sprite->getRenderHeight())
{
	this->sprite = sprite;
	hp = 2;
	cooldown = 1200; // pequena espera para nao atirar no instante em que nasce
	angle = 0;       // cano para cima
}

Turret::~Turret()
{
	// A torreta e a unica dona da sua sprite (cada uma rotaciona a sua).
	delete(sprite);
	sprite = NULL;
}

void Turret::update(int dt)
{
	if(cooldown > 0)
		cooldown -= dt;
}

// Convencao identica a da nave (AccObject): angulo 0 = cano para cima,
// +90 = para a esquerda. Dado o vetor (dx, dy) ate o alvo em coordenadas
// de tela (y cresce para baixo): angulo = 90 + atan2(dy, -dx).
void Turret::aimAt(float targetX, float targetY)
{
	float cx = x + w / 2;
	float cy = y + h / 2;
	float dx = targetX - cx;
	float dy = targetY - cy;

	float angulo = 90.0f + atan2f(dy, -dx) * 180.0f / TURRET_PI;
	angle = angulo;
}

void Turret::render(float cameraX, float cameraY)
{
	if(!sprite)
		return;

	// O rotoZoom so refaz a surface quando o angulo muda.
	sprite->rotoZoom(angle);
	sprite->render(x - cameraX, y - cameraY);
}
