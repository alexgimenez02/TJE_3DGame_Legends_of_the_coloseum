#ifndef STAGE_H
#define STAGE_H

#include "utils.h"
#include "entity.h"
#include "extra/textparser.h"
#include "fbo.h"
#include "enemyAI.h"
#include <bass.h>
#include "animation.h"


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
typedef struct CHECK_LIST {
	bool first = true;
	bool second = false;
	bool third = false;
};
class GameStage : public Stage
{
public:
	
	bool toggle = false;
	CHECK_LIST list;
	OBJECTIVE obj = TUTORIAL; 
	vector<Texture*> textures;
	sWeapon weapon;
	EntityMesh* player; //Has no real mesh, just to get the model
	vector<EntityMesh*> enemies;
	vector<EntityMesh*> entities, entitiesColision;
	EntityMesh* UIArrow;
	EntityMesh* barTender;
	EntityMap* sky;
	EntityMap* terrain;
	EntityMesh* sphereTabern, *sphereArena;
	STAGE_ID Stage_ID = MAP, previous_stage = MAP;
	Shader* shader, *gui_shader;
	sSTATS stats;
	EnemyAI* currentEnemy;
	HSAMPLE currentSFX;
	DWORD curr_SFX_channel;
	POSITION Attack = NONE;
	/*vector<Mesh*> idle_mesh, up_mesh, right_mesh, left_mesh, down_mesh;
	vector<Animation*> idle_attack, up_attack, right_attack, left_attack, down_attack;*/
	float durationTime = 0.0f;
	float cooldown = 2.0f;
	float parry = 0.0f;
	bool mapSwap = true, yAxisCam = false, isBattle = false, debug = false, parried = false, menu = false, interaction = false, lvlUpMenu = false;
	int nextText = 0;
	int lvl = 0, diff = 0;
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



