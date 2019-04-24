#ifndef DRAWABLEOBJECT_H
#define DRAWABLEOBJECT_H

#include "sceneElements.h"

class drawableObject
{
public:
	virtual void draw() = 0;
	virtual void setGui() {};

	//if the class will cointain some logic, so it must be refreshed at each game loop cycle by calling update. Otherwise just don't override it.  
	virtual void update() {};

	static sceneElements* scene;
};

#endif