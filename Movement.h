#ifndef MOVEMENT_H
#define MOVEMENT_H

#include <vecmath.h>

class Movement 
{
public:
	Movement();
	typedef enum { NONE, LEFT, RIGHT } Button;

	void moveRight(float angle, const Matrix4f &m, Button button);
	void moveLeft(float angle, const Matrix4f &m, Button button);

protected:


};

#endif
