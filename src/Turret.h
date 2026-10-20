/*
 * Turret.h
 *
 *  Created on: 20/10/2026
 *      Author: Victor
 *
 *  Torreta fixa ancorada nos paineis solidos do mapa. Aponta para a nave e
 *  atira quando ela esta ao alcance (a linha de tiro e bloqueada por tiles).
 *  A cadencia, o alvo e os tiros vivem no StateGame; aqui so o objeto.
 */

#ifndef TURRET_H_
#define TURRET_H_

#include "GameObject.h"
#include "Sprite.h"

class Turret: public GameObject
{
	private:
		Sprite *sprite;
	public:
		int hp;         // 2 tiros para destruir
		int cooldown;   // ms ate o proximo disparo
		float angle;    // graus, usado no rotoZoom da sprite (cano para cima em 0)

		Turret(Sprite* sprite, float x, float y);
		virtual ~Turret();
		virtual void update(int dt);
		void render(float cameraX, float cameraY);
		void aimAt(float targetX, float targetY);
};

#endif /* TURRET_H_ */
