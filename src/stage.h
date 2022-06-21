#ifndef STAGE_H
#define STAGE_H

#include "utils.h"
#include "entity.h"
#include "extra/textparser.h"
#include "enemyAI.h"


typedef struct COLISION_RETURN {
	bool colision;
	Vector3 modifiedPosition;
}COL_RETURN;
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
	MAP = 0,
	TABERN = 1,
	ARENA = 2
};
typedef struct WEAPON_STRUCT {
	Vector3 weaponOffset = Vector3(-0.15f, 0.25f, 0.4f);
	float attackMotion = 0.0f;
	float defenceMotion = 0.0f, defenceMotionUp = 0.0f, defenceRotation = 0.0f;
	POSITION defType = NONE; //What type of defence move is being used
	bool movementMotion = false; //To animate when moving
	bool attack = false, down = true, attacked = false; //Attack button is pressed
	bool defence = false; //Defence button pressed
	bool defMotion = false, defMotionUp = false, defRotation = false; //Different defence action checkers
	EntityMesh* entity;
}sWeapon;
class GameStage : public Stage
{
public:
	
	sWeapon weapon;
	EntityMesh* player; //Has no real mesh, just to get the model
	vector<EntityMesh*> enemies;
	vector<EntityMesh*> entities, entitiesColision;
	EntityMesh* barTender;
	EntityMap* sky;
	EntityMap* terrain;
	EntityMesh* sphereTabern, *sphereArena;
	STAGE_ID Stage_ID = MAP;
	Shader* shader;
	EnemyAI* currentEnemy;
	bool mapSwap = true, yAxisCam = false, isBattle = false, debug = false;
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



