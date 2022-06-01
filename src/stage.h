#ifndef STAGE_H
#define STAGE_H

#include "utils.h"
#include "entity.h"
#include "extra/textparser.h"

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

enum STAGE_ID {
	MAP = 1,
	TABERN = 2,
	ARENA = 3
};

class GameStage : Stage
{
public:

	EntityMesh* player; //Has no real mesh, just to get the model
	vector<EntityMesh*> entities;
	EntityMap* sky;
	EntityMap* terrain;
	STAGE_ID Stage_ID;
	bool mapSwap = false, yAxisCam = false;
	float tiling = 20.0f;


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



