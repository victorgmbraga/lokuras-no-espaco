/*
 * Animation.cpp
 *
 *  Created on: 05/06/2013
 *      Author: Victor
 */

#include "Animation.h"

Animation::Animation(string filename, int frameTime, int n_sprites): Sprite(filename)
{
	this->frameTime = frameTime;
	this->n_sprites = n_sprites;
	frameSkip = 0;
	time = 0;

	clipRect.w = surface->w/n_sprites;
	clipRect.h = surface->h;
}

Animation::~Animation()
{
	// TODO Auto-generated destructor stub
}

void Animation::update(int dt)
{
	// Some dt ao tempo
	time += dt;

	// Divida a variável do tempo pelo valor do tempo do frame, e o quociente da
	// divisão deve ser atribuído à variável saltoFrame, que é justamente o número
	// de frames que devem ser saltados.
	// A explicação é a seguinte: consideremos que o tempo de frame seja 30. Se a
	// variável de tempo estiver entre 30 e 59, significa que teremos de pular um
	// frame. Agora se estiver entre 60 e 89, significa que teremos de pular dois
	// frames.
	int saltoFrame = time/frameTime;

	// O resto da divisão deve ser atribuído a própria variável de tempo.
	// A explicação é a seguinte: suponha que o tempo de frame seja 30. Se a
	// variável de tempo estiver em 30, significa que saltoFrame deverá ser 1 e o
	// tempo deve ser resetado para 0, que é justamente o resto da divisão. Se for
	// 35, por exemplo, o saltoFrame será 1 e o tempo será resetado para 5.
	// No caso de o tempo ser menor do que 30, como 20, por exemplo, o resto
	// continuará sendo 20, então isso não é problema.
	time = time % frameTime;

	// O valor de clipRect.x deve ser incrementado em clipRect.w*saltoFrames, o
	// que fará com que o srcrect mova para frente, seguindo para a próxima
	// animação, saltando um número “saltoFrames” de vezes.
	clipRect.x += clipRect.w * saltoFrame;

	// Se clipRect.x >= surface->w, então significa que o srcrect passou do espaço
	// da sheet. Então, devemos fazer clipRect.x = (clipRect.x - surface->w) %
	// surface->w, ou seja, o valor de x de clipRect retorna para a posição
	// apropriada.
	if(clipRect.x >= surface->w)
		clipRect.x = (clipRect.x - surface->w) % surface->w;

	// Por fim, fazemos o clip da imagem na posição calculada.
	clip(clipRect.x, clipRect.y, clipRect.w, clipRect.h);
}

void Animation::setFrame(int frame)
{
	// Método para definir o próximo sprite a ser renderizado.
	// Para isso, basta ajustar clipRect.x para a posicao desejada (clipRect.w *
	// frame) e fazer o clip.
	clipRect.x = clipRect.w * frame;

	if(clipRect.x >= surface->w)
		clipRect.x = (clipRect.x - surface->w) % surface->w;

	clip(clipRect.x, clipRect.y, clipRect.w, clipRect.h);
}

void Animation::setFrameTime(int time)
{
	// Redefine o valor de frametime.
	if(time >= 0)
		frameTime = time;
}

int Animation::getFrameTime()
{
	return frameTime;
}

