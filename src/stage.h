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
	Shader* a_shader;
	float iconx = 0.0f, icony = 0.0f;
	int num_iconfiles = 0;
	vector<string> icons;
	vector<ICON_POSITION> positions;
	vector<Texture*> textures, textures_hover;
	EntityMap* terrain, *sky;
	EntityMesh* colosseum;
	Camera* cam;
	//imagen de fondo para el menu

	void render();
	void update(float elapsed_time);
	void reloadIcons();

};

class ControlsStage : public Stage
{
public:

	vector<string> icons;
	vector<Texture*> textures;
	vector<ICON_POSITION> positions;

	Shader* a_shader;
	EntityMap* terrain, * sky;
	EntityMesh* colosseum;
	Camera* cam;

	//imagen de fondo + mesh de los controles

	void render();
	void update(float elapsed_time);
	void savePosition(float x, float y, const char* filename);
	void ReloadIcons();


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
	
	vector<Texture*> textures;
	sWeapon weapon;
	EntityMesh* player; //Has no real mesh, just to get the model
	vector<EntityMesh*> enemies;
	vector<EntityMesh*> entities, entitiesColision;
	EntityMesh* barTender;
	EntityMap* sky;
	EntityMap* terrain;
	EntityMesh* sphereTabern, *sphereArena;
	STAGE_ID Stage_ID = MAP, previous_stage = MAP;
	Shader* shader, *gui_shader;
	sSTATS stats;
	EnemyAI* currentEnemy;
	bool mapSwap = true, yAxisCam = false, isBattle = false, debug = false, parried = false, menu = false;
	float tiling = 20.0f, waitTime = 0.0f;

	void render();
	void update(float elapsed_time);

};

class GameOverStage : public Stage
{
public:
	Camera* cam;
	vector<EntityMesh*> entities, enemies;
	EntityMap* sky, *terrain;
	STAGE_ID Stage_ID;
	Shader* shader, *gui_shader;
	float tiling = 20.0f;

	//imagen de fondo con texto de gameOver
	void render();
	void update(float elapsed_time);

};

#endif // !STAGE_H



