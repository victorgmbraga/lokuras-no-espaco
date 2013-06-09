/*
 * GameObject.cpp
 *
 *  Created on: 26/05/2013
 *      Author: Victor
 */

#include "GameObject.h"

GameObject::GameObject(float x, float y, float w, float h)
{
	this->x = x;
	this->y = y;
	this->w = w;
	this->h = h;
	camera = NULL;
	inputManager = NULL;
}

bool GameObject::collidesWith(GameObject* other)
{
	Rect rectA;
	rectA.x = x;
	rectA.y = y;
	rectA.w = w;
	rectA.h = h;

	Rect rectB;
	rectB.x = other->x;
	rectB.y = other->y;
	rectB.w = other->w;
	rectB.h = other->h;

	// Para calcular a colisão entre dois retângulos, faremos uma série de testes
	// para verificar que eles não interceptam, cobrindo todas as possibilidades. Se todos
	// estes testes falharem, significa que ocorreu a intercepção.

	// Sejam dois retângulos A = (XA, YA, dxA, dyA) e B = (XB, YB, dxB, dyB).
	// diffX= XA – XB e diffY = YA – YB.
	float diffX = rectA.x - rectB.x;
	float diffY = rectA.y - rectB.y;

	// Se diffX > dxB ou -diffX > dxA, retorna falso.
	if(diffX > rectB.w || -diffX > rectA.w)
		return false;
	// Se diffY > dyB ou -diffY > dyA, retorna falso.
	else if(diffY > rectB.h || -diffY > rectA.h)
		return false;
	// Se falhar em todos estes testes, então retorna verdadeiro.
	else
		return true;
}
