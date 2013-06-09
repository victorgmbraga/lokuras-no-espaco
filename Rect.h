/*
 * Rect.h
 *
 *  Created on: 07/06/2013
 *      Author: Victor
 */

#ifndef RECT_H_
#define RECT_H_

class Rect
{
	public:
		float x, y;
		float w, h;
		Rect(float x = 0, float y = 0, float w = 0, float h = 0): x(x), y(y), w(w), h(h) { }
		//virtual ~Rect();
};

#endif /* RECT_H_ */
