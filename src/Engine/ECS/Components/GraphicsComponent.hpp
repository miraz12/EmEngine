#include "Component.hpp"

#include "Objects/GraphicsObject.hpp"

class GraphicsComponent : public Component 
{

public:
	GraphicsObject* grapObj;

	GraphicsComponent();
	virtual ~GraphicsComponent();
};
