#ifndef DRAWABLEOBJECT_H
#define DRAWABLEOBJECT_H

#include "sceneElements.h"

class drawableObject
{
public:
	virtual void draw() = 0;
	virtual void setGui() {};

	static sceneElements* scene;
};

#endif