#pragma once
#include "../Engine/Window.h"
#include "drawableObject.h"

#include <list>

class GUI : public drawableObject
{
public:
	GUI(Window& w);
	~GUI();

	virtual void draw();
	virtual void update();

	GUI& subscribe(drawableObject* subscriber);

private:
	// the other drawableObjects expose their setGui() methods (because he let the user handle their own attributes), so they can be subscribed to the GUI class
	// the GUI class will call the setGui() method for each subscriber
	std::list<drawableObject*> subscribers;

};

