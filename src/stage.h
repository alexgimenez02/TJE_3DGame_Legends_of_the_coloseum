#ifndef STAGE_H
#define STAGE_H

#include "utils.h"
#include "entity.h"

class Stage
{
public:

	//Atributes
	string name;


	Stage(); //Constructor
	virtual ~Stage(); //Destructor


	//Methods
	virtual void render() = 0;
	virtual void update(float elapsed_time) = 0;

};

class IntroStage : Stage
{
public:
	
	//imagen de fondo para el menu

	void render();
	void update(float elapsed_time);

};

class ControlsStage : Stage
{
public:

	//imagen de fondo + mesh de los controles

	void render();
	void update(float elapsed_time);

};

class GameStage : Stage
{
public:

	EntityMap map;
	vector<EntityMesh> entities;

	void render();
	void update(float elapsed_time);

};

class GameOverStage : Stage
{
public:

	//imagen de fondo con texto de gameOver
	void render();
	void update(float elapsed_time);

};

#endif // !STAGE_H



