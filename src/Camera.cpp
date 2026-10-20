/*
 * Camera.cpp
 *
 *  Created on: 28/05/2013
 *      Author: Victor
 */

#include "Camera.h"

Camera* Camera::instance = NULL;

Camera::Camera(float x, float y, GameObject *bindObject)
{
	this->x = x;
	this->y = y;
	this->bindObject = bindObject;
	this->boundW = 0;
	this->boundH = 0;
	this->hasBounds = false;
}

Camera& Camera::getInstance()
{
	if (!instance)
		instance = new Camera();
	return *instance;
}

void Camera::update(int dt)
{
	// Se a câmera está associada com um GameObject, persegue o centro
	// dele com lookahead na direcao da velocidade (antecipa a acao) e
	// aproximacao suave (sem solavanco a cada frame).
	if(bindObject)
	{
		float alvoX = bindObject->x - SCREEN_W/2 + bindObject->w/2
		              + bindObject->getVelX() * 350.0f;
		float alvoY = bindObject->y - SCREEN_H/2 + bindObject->h/2
		              + bindObject->getVelY() * 350.0f;

		float k = dt * 0.006f;
		if(k > 1.0f)
			k = 1.0f;
		x += (alvoX - x) * k;
		y += (alvoY - y) * k;

		clampToBounds();
	}
	// Senão, atualiza a câmera de acordo com as setas do teclado.
	// (usada livremente apos o fim da partida)
	else
	{
		InputManager &im = InputManager::getInstance();

		if(im.isKeyPressed(SDLK_UP))
			y -= CAMERA_SPEED * dt;
		if(im.isKeyPressed(SDLK_DOWN))
			y += CAMERA_SPEED * dt;
		if(im.isKeyPressed(SDLK_LEFT))
			x -= CAMERA_SPEED * dt;
		if(im.isKeyPressed(SDLK_RIGHT))
			x += CAMERA_SPEED * dt;

		clampToBounds();
	}
}

void Camera::bind(GameObject* bindObject)
{
	this->bindObject = bindObject;

	// Encaixe imediato no alvo (sem deslizar da origem): a suavizacao
	// vale para o movimento, nao para a troca de estado.
	if(bindObject)
	{
		x = bindObject->x - SCREEN_W/2 + bindObject->w/2;
		y = bindObject->y - SCREEN_H/2 + bindObject->h/2;
		clampToBounds();
	}
}

void Camera::unBind()
{
	bindObject = NULL;
}

void Camera::setBounds(int w, int h)
{
	boundW = w;
	boundH = h;
	hasBounds = true;
	clampToBounds();
}

void Camera::clearBounds()
{
	hasBounds = false;
}

void Camera::clampToBounds()
{
	if(!hasBounds)
		return;

	// Mundo menor que a tela: centraliza (mostra as bordas como antes).
	if(boundW <= SCREEN_W)
		x = (boundW - SCREEN_W) / 2.0f;
	else if(x < 0)
		x = 0;
	else if(x > boundW - SCREEN_W)
		x = (float)(boundW - SCREEN_W);

	if(boundH <= SCREEN_H)
		y = (boundH - SCREEN_H) / 2.0f;
	else if(y < 0)
		y = 0;
	else if(y > boundH - SCREEN_H)
		y = (float)(boundH - SCREEN_H);
}
