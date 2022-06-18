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

	/*
	Stage(); //Constructor
	virtual ~Stage(); //Destructor
	*/


	//Methods
	virtual void render() = 0;
	virtual void update(float elapsed_time) = 0;

};

class IntroStage : public Stage
{
public:
	
	//imagen de fondo para el menu

	void render();
	void update(float elapsed_time);

};

class ControlsStage : public Stage
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
typedef struct WEAPON_STRUCT {
	Vector3 weaponOffset = Vector3(-0.05f, 0.25f, 2.6f);
	float attackMotion = 0.0f;
	float defenceMotion = 0.0f, defenceMotionUp = 0.0f, defenceRotation = 0.0f;
	POSITION defType = NONE; //What type of defence move is being used
	bool movementMotion = false; //To animate when moving
	bool attack = false, down = true; //Attack button is pressed
	bool defence = false; //Defence button pressed
	bool defMotion = false, defMotionUp = false, defRotation = false; //Different defence action checkers
	EntityMesh* entity;
}sWeapon;
class GameStage : public Stage
{
public:
	
	sWeapon weapon;
	EntityMesh* player; //Has no real mesh, just to get the model
	vector<EntityMesh*> entities;
	EntityMap* sky;
	EntityMap* terrain;
	STAGE_ID Stage_ID;
	Shader* shader;
	bool mapSwap = false, yAxisCam = false, isBattle = false;
	float tiling = 20.0f;

	void render();
	void update(float elapsed_time);

};

class GameOverStage : public Stage
{
public:

	//imagen de fondo con texto de gameOver
	void render();
	void update(float elapsed_time);

};

#endif // !STAGE_H



