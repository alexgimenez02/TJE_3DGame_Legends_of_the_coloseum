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
	//some attributes
	string name;
	Matrix44 model;

	//pointer to my parent entity
	Entity* parent;
	//pointers to my children
	vector<Entity*> children;

	Entity(){}; //constructor
	virtual ~Entity() {}; //destructor

	//methods overwritten by derived classes
	virtual void render() = 0;
	virtual void update(float elapsed_time) = 0;

	Matrix44 getGlobalMatrix(); //returns transform in world coordinates
	void destroy(); //destroy the entity and its children

};



class EntityMesh : public Entity
{
public:
	string name;
	Mesh* mesh;
	Texture* texture;
	Shader* shader;
	Vector3 pos;
	float scale;
	float pitch;
	float yaw;
	int id;

	EntityMesh(){
		name = "";
		//scale = Vector3();
		scale = 0.0f;
		pos = Vector3();
		pitch = 0.0f;
		yaw = 0.0f;
	};
	~EntityMesh(){};
	//methods overwritten
	void render();
	void update(float elapsed_time);
};


class EntityMap : public Entity
{
public:
	Mesh* mesh;
	Texture* texture;
	Shader* shader;
	Vector3 scale;

	EntityMap() {};
	~EntityMap() {};
	//Methods overwritten
	void render();
	void update(float elapsed_time);	
};
#endif // !ENTITY_H