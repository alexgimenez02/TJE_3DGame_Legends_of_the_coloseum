#ifndef ENTITY_H
#define ENTITY_H

#include "utils.h"
#include "mesh.h"
#include "texture.h"
#include "camera.h"
#include "shader.h"
#include "assert.h"

class Entity
{
public:
	//Entity(); //constructor
	//virtual ~Entity(); //destructor

	//some attributes
	string name;
	Mesh* mesh;
	Matrix44 model;
	Texture* texture;
	Shader* shader;
	Vector4 color;
	Vector3 scale;

	//pointer to my parent entity
	Entity* parent;
	//pointers to my children
	std::vector<Entity*> children;

	//methods overwritten by derived classes
	virtual void render();
	virtual void update(float elapsed_time);

	//methods
	void addChild(Entity* ent);
	void removeChild(Entity* ent);

	Matrix44 getGlobalMatrix(); //returns transform in world coordinates
	void destroy(); //destroy the entity and its children
	Vector3 getPosition();

};



class EntityMesh : Entity
{
public:
	Mesh* mesh;
	Matrix44 model;
	Texture* texture;
	Shader* shader;
	Vector4 color;
	Vector3 scale;

	//methods overwritten
	void render();
	void update(float elapsed_time);
};

#endif // !ENTITY_H