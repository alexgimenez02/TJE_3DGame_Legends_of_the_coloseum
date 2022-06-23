#include "game.h"
#include "utils.h"
#include "mesh.h"
#include "texture.h"
#include "fbo.h"
#include "shader.h"
#include "input.h"
#include "animation.h"
#include <Windows.h>
#include "extra/textparser.h"
#include <cmath>

//some globals
Mesh* mesh = NULL;
Texture* tex = NULL;
Shader* shader = NULL;
Animation* anim = NULL;
float angle = 0, padding = 10.0f;
float mouse_speed = 1.0f;
FBO* fbo = NULL;
float loadDistance = 200.0f;
float no_render_distance = 1000.0f;
bool cameraLocked = false, yAxisCam = false, checkCol = false, editorMode = false, attack = false, down = true, movementMotion = false, defence = false;
bool defMotion = false, defMotionUp = false, defRotation = false;
float colisionRadius = 2.0f, attackMotion = 0.0f, defenceMotion = 0.0f, defenceMotionUp = 0.0f, defenceRotation = 0.0f;
POSITION defType = NONE;



bool meshSwap = false;
int currMeshIdx = 0;
vector<string> meshnames, texnames;
string currentMesh = "data/editor/barn.obj", currentTex = "data/editor/materials.tga";

EntityMesh* player;
EntityMesh* sword;
EntityMap* terrain;
EntityMap* sky;

Mesh* groundMesh;
Texture* groundTex;
vector<EntityMesh*> meshes;
Game* Game::instance = NULL;
EntityMesh* SelectedEntity;
bool mapSwap = false;


Game::Game(int window_width, int window_height, SDL_Window* window)
{
	this->window_width = window_width;
	this->window_height = window_height;
	this->window = window;
	instance = this;
	must_exit = false;

	fps = 0;
	frame = 0;
	time = 0.0f;
	elapsed_time = 0.0f;
	mouse_locked = false;

	//OpenGL flags
	glEnable( GL_CULL_FACE ); //render both sides of every triangle
	glEnable( GL_DEPTH_TEST ); //check the occlusions using the Z buffer

	intro = new IntroStage();
	controls = new ControlsStage();
	game_s = new GameStage();
	gameOver = new GameOverStage();

	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");
	//create our camera
	camera = new Camera();
	camera->lookAt(Vector3(0.f,100.f, 100.f),Vector3(0.f,0.f,0.f), Vector3(0.f,1.f,0.f)); //position the camera and point to 0,0,0
	camera->setPerspective(70.f,window_width/(float)window_height,0.1f,10000.f); //set the projection, we want to be perspective


	//load one texture without using the Texture Manager (Texture::Get would use the manager)
	//LoadSceneFile("data/MapJordiAlex.scene");
	/* { //Sword Mesh
		sword->mesh = Mesh::Get("data/props/sword.obj");
		sword->texture = Texture::Get("data/textures/sword.png");
		sword->model.scale(1.0f / 20.0f, 1.0f / 20.0f, 1.0f / 20.0f);
	}*/
	
	//Game Stage Init
	game_s->sky = new EntityMap();
	game_s->sky->mesh = Mesh::Get("data/cielo.ASE");
	game_s->terrain = new EntityMap();
	game_s->sky->texture = new Texture();
	game_s->sky->texture->load("data/cielo.tga");
	game_s->terrain->mesh = new Mesh();
	game_s->terrain->mesh->createPlane(100);
	game_s->terrain->texture = Texture::Get("data/grass.tga");

	game_s->textures.push_back(Texture::Get("data/gameIcons/hp_bar_frame.png"));
	game_s->textures.push_back(Texture::Get("data/gameIcons/hp_bar.png"));

	game_s->stats = {
		2,
		0.0f,
		0.9f
	};
	//Intro Stage Init
	intro->a_shader = Shader::Get("data/shaders/basic.vs", "data/shaders/gui.fs");
	intro->icons = get_all_files_names_within_icons();
	intro->num_iconfiles = intro->icons.size();
	for (size_t i = 0; i < intro->num_iconfiles; i++)
	{
		intro->positions.push_back(readPosition(intro->icons[i].c_str())); //controlsIconsTextures
	}
	intro->textures.push_back(Texture::Get("data/iconTextures/Play Button.png"));
	intro->textures.push_back(Texture::Get("data/iconTextures/Controls Button.png"));
	intro->textures.push_back(Texture::Get("data/iconTextures/Exit Button.png"));

	intro->textures_hover.push_back(Texture::Get("data/iconTextures/Play hover.png"));
	intro->textures_hover.push_back(Texture::Get("data/iconTextures/Controls  hover.png"));
	intro->textures_hover.push_back(Texture::Get("data/iconTextures/Exit hover.png"));

	//Background
	//Terrain
	intro->terrain = new EntityMap();
	intro->terrain->mesh = new Mesh();
	intro->terrain->mesh->createPlane(7000);
	intro->terrain->texture = Texture::Get("data/grass.tga");
	//Sky
	intro->sky = new EntityMap();
	intro->sky->mesh = Mesh::Get("data/cielo.ASE");
	intro->sky->texture = Texture::Get("data/cielo.tga");
	intro->sky->shader = shader;

	//Colosseum
	intro->colosseum = new EntityMesh();
	intro->colosseum->mesh = Mesh::Get("data/props/Coliseo.obj");
	intro->colosseum->texture = Texture::Get("data/textures/Coliseo.png");

	//Camera
	intro->cam = new Camera();
	intro->cam = Game::instance->camera;
	intro->cam->lookAt(Vector3(148.92f, 77.76f, 57.58f), Vector3(30.0f, 21.99f, 9.88f), Vector3(0, 1, 0));

	//Controls stage init
	controls->a_shader = intro->a_shader;
	//Sky
	controls->sky = intro->sky;


	//Terrain
	controls->terrain = intro->terrain;

	//Colosseum
	controls->colosseum = intro->colosseum;

	//Cam
	controls->cam = new Camera();
	controls->cam->lookAt(Vector3(148.92f, 77.76f, 57.58f), Vector3(30.0f, 21.99f, 9.88f), Vector3(0, 1, 0));

	controls->icons = get_all_files_names_within_folder();
	for (size_t i = 0; i < controls->icons.size(); i++)
	{
		controls->positions.push_back(readPosition(controls->icons[i].c_str()));
	}
	controls->textures.push_back(Texture::Get("data/controlsIconsTextures/box.png"));

	
	// example of shader loading using the shaders manager
	game_s->shader = new Shader();
	game_s->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");
	game_s->gui_shader = Shader::Get("data/shaders/basic.vs", "data/shaders/gui.fs");

	game_s->sky->shader = game_s->shader;
	game_s->terrain->shader = game_s->shader;
	game_s->player = new EntityMesh();
	game_s->weapon.entity = new EntityMesh();
	game_s->weapon.entity->mesh = Mesh::Get("data/props/sword.obj");
	game_s->weapon.entity->texture = Texture::Get("data/textures/sword.png");
	game_s->weapon.entity->scale = 1 / 20.0f;

	current_stage = intro;
	SDL_ShowCursor(!mouse_locked); //hide or show the mouse
}


//what to do when the image has to be draw
void Game::render(void)
{
	//set the clear color (the background color)
	glClearColor(0.0, 0.0, 0.0, 1.0);

	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	current_stage->render();

	//set flags
	/*
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	//create model matrix for cube
	//Camera* cam = Game::instance->camera;
	
	if (!meshes.empty())
	{
		for (size_t i = 0; i < meshes.size(); i++)
		{
			//RenderMesh(meshes[i]->model, meshes[i]->mesh, meshes[i]->texture, shader, cam);
		}
	}
	Matrix44 playerModel;
	playerModel.translate(player.pos.x, player.pos.y, player.pos.z);
	playerModel.rotate(player.yaw * DEG2RAD, Vector3(0, 1, 0));
	//set the camera as default
	//Parentar model --> Offset respecte player que defineix on esta l'arma
	Matrix44 swordModel;
	Matrix44 camModel;
	if (cameraLocked) {
		camModel = playerModel;
		camModel.rotate(player.pitch * DEG2RAD, Vector3(1, 0, 0));
		Vector3 eye = playerModel * Vector3(0.1f, 0.45f, 2.25f);
		Vector3 center = camera->center;
		if(!yAxisCam)
			center = eye + camModel.rotateVector(Vector3(0,0,1));
		Vector3 up = camModel.rotateVector(Vector3(0,1,0));
		camera->enable();
		camera->lookAt(eye, center, up);
	}
	Vector3 swordOffset = Vector3(-0.05f,0.25f ,2.6f);
	swordModel.setTranslation(swordOffset.x, swordOffset.y, swordOffset.z);
	swordModel.rotate(-90.0f * DEG2RAD, Vector3(0, 1, 0));
	swordModel.scale(1 / 25.0f, 1 / 25.0f, 1 / 25.0f);
	swordModel = swordModel * playerModel;
	if (defType != NONE)
	{
		if(defType == UP)
		{
			//motion defence up
			swordModel.translate(0.0f, defenceMotionUp, defenceMotion);
			swordModel.rotate(defenceRotation * DEG2RAD, Vector3(1, 0, 0));
		}
		else
		{
			swordModel.translate(0.0f, 0.0f, defenceMotion);
		}
	}
	if (movementMotion)
	{
		Matrix44 T;
		T.setTranslation(0.0f, 0.025f * sin(time) + 0.05f, 0.0f);
		swordModel = swordModel * T;
	}
	swordModel.rotate(attackMotion * DEG2RAD, Vector3(0, 0, 1));
	*/
	//RenderMesh(swordModel, sword.mesh, sword.texture, shader, cam);
	//RenderPlane(20.0f);

	//render the FPS, Draw Calls, etc
	drawText(2, 2, getGPUStats(), Vector3(1, 1, 1), 2);
	//swap between front buffer and back buffer
	//drawCrosshair();
	
	wasLeftButtonPressed = false;
	SDL_GL_SwapWindow(this->window);
}
//Adding entities
void AddEntityInFront(Camera* cam, const char* meshName, const char* texName)
{
	Vector2 mouse = Input::mouse_position;
	Game* g = Game::instance;
	Vector3 dir = cam->getRayDirection(mouse.x, mouse.y, g->window_width, g->window_height);
	Vector3 rayOrigin = cam->eye;

	Vector3 spawnPos = RayPlaneCollision(Vector3(), Vector3(0, 1, 0), rayOrigin, dir);
	Matrix44 model;
	model.translate(spawnPos.x, spawnPos.y, spawnPos.z);
	
	EntityMesh* entity = new EntityMesh();
	entity->mesh = Mesh::Get(meshName); 
	entity->name = meshName;
	entity->model = model;	
	entity->texture = Texture::Get(texName);
	meshes.push_back(entity);
}


//Checking  ray colision
void CheckCol(Camera* cam)
{
	Vector2 mouse = Input::mouse_position;
	Game* g = Game::instance;
	Vector3 dir = cam->getRayDirection(mouse.x, mouse.y, g->window_width, g->window_height);
	Vector3 rayOrigin = cam->eye;
	for (size_t i = 0; i < meshes.size(); i++)
	{
		EntityMesh* entity = meshes[i];
		Vector3 pos;
		Vector3 normal;
		if (entity->mesh->testRayCollision(entity->model, rayOrigin, dir, pos, normal))
		{
			SelectedEntity = entity;
			cout << "col" << endl;
			break;
		} 
	}
}

void RotateSelected(float angleDegrees)
{
	if (SelectedEntity == NULL) {
		return;
	}
	SelectedEntity->model.rotate(angleDegrees * DEG2RAD, Vector3(0, 1, 0));
}
void Game::update(double seconds_elapsed)
{

	current_stage->update(seconds_elapsed);
	switch (scene)
	{
	case INTRO:
		current_stage = intro;
		
		SDL_ShowCursor(true);
		break;
	case CONTROLS:
		current_stage = controls;
		break;
	case GAME:
		current_stage = game_s;
		break;
	case GAMEOVER:
		current_stage = gameOver;
		break;
	}
}

//Keyboard event handler (sync input)
void Game::onKeyDown( SDL_KeyboardEvent event )
{
	switch(event.keysym.sym)
	{
		case SDLK_ESCAPE:  
			if (current_stage == game_s) {
				GameStage* stg = (GameStage*)current_stage;
				stg->menu = !stg->menu;
			}
			else{
				scene = INTRO;
				intro->cam->lookAt(Vector3(148.92f, 77.76f, 57.58f), Vector3(30.0f, 21.99f, 9.88f), Vector3(0, 1, 0)); 
			}
			break; //ESC key, kill the app
		case SDLK_F1: Shader::ReloadAll(); break; 
		case SDLK_F2: 
			cout << "Camera positions: \nEye: (" << camera->eye.x << "," << camera->eye.y << "," << camera->eye.z << ")\n"
				<< "Center: (" << camera->center.x << ", " << camera->center.y << ", " << camera->center.z << ")\n"
				<< "Up: (" << camera->up.x << ", " << camera->up.y << ", " << camera->up.z << ")\n" << endl;
			break;
		case SDLK_F3:
			checkCol = !checkCol;
			if (checkCol) cout << "Checking colisions!" << endl;
			else cout << "Adding entities!" << endl;
			break;
		case SDLK_F4:
			editorMode = !editorMode;
			if (editorMode) {
				cout << "Editor mode on" << endl;
				cout << "Available meshes:" << endl;
				for (size_t i = 0; i < meshnames.size(); i++)
				{
					cout << i << ") " << meshnames[i] << endl;
				}
			}
			break;
		case SDLK_KP_PLUS: RotateSelected(10.0f); break;
		case SDLK_KP_MINUS: RotateSelected(-10.0f); break;
		case SDLK_RIGHT:
			if (current_stage == game_s) {
				GameStage* stg = (GameStage*)current_stage;
				if (stg->Stage_ID != TABERN) {
					if (!defence)
					{
						stg->weapon.defence = true; 
						stg->weapon.defType = RIGHT;
						stg->weapon.defMotion = true;
					}
				}
			}
			break;
		case SDLK_LEFT: 
			if (current_stage == game_s) {
				GameStage* stg = (GameStage*)current_stage;
				if (stg->Stage_ID != TABERN) {
					if (!defence)
					{
						stg->weapon.defence = true;
						stg->weapon.defType = LEFT;
						stg->weapon.defMotion = true;
					}
				}
			}
				break;
		case SDLK_UP:
			if (current_stage == game_s) {
				GameStage* stg = (GameStage*)current_stage;
				if (stg->Stage_ID != TABERN)
				{
					if (!defence)
					{
						stg->weapon.defence = true;
						stg->weapon.defType = UP;
						stg->weapon.defMotion = true;
						stg->weapon.defMotionUp = true;
						stg->weapon.defRotation = true;
					}

				}
			}
			break;
		case SDLK_0:
			if (current_stage == game_s)
			{
				GameStage* stg = (GameStage*)current_stage;
				stg->mapSwap = true;
				stg->Stage_ID = (STAGE_ID)((stg->Stage_ID + 1) % 3);
			}
			break;
		case SDLK_F6:
			if (current_stage == game_s)
			{
				GameStage* stg = (GameStage*)current_stage;
				stg->debug = !stg->debug;
			}
			break;
		case SDLK_F7:
			if (current_stage == game_s)
			{
				GameStage* stg = (GameStage*)current_stage;
				cout << "Player yaw: ( " << stg->player->yaw << ")" << endl;
				cout << "Player position: ( " << stg->player->pos.x << ","<< stg->player->pos.y << "," << stg->player->pos.z << ")" << endl;
			}
	} 
}

void Game::onKeyUp(SDL_KeyboardEvent event)
{
}

void Game::onGamepadButtonDown(SDL_JoyButtonEvent event)
{

}

void Game::onGamepadButtonUp(SDL_JoyButtonEvent event)
{

}

void Game::onMouseButtonDown( SDL_MouseButtonEvent event )
{
	if (event.button == SDL_BUTTON_MIDDLE) //middle mouse
	{
		mouse_locked = !mouse_locked;
		SDL_ShowCursor(!mouse_locked);
	}
	if (event.button == SDL_BUTTON_RIGHT)
	{
		if (!checkCol)
		{
			if (editorMode)
			{
				AddEntityInFront(camera, currentMesh.c_str(), currentTex.c_str());
				cout << "Object added" << endl;
			}
		}
		else
		{
			CheckCol(camera);
		}
	}
	if (event.button == SDL_BUTTON_LEFT)
	{
		if (current_stage == game_s)
		{
			GameStage* stg = (GameStage*)current_stage;
			if (stg->menu) {
				wasLeftButtonPressed = true;
			}
			else {
				if (stg->Stage_ID != TABERN) {
					if(stg->isBattle) stg->weapon.attacked = true;
					stg->weapon.attack = true; 
					stg->weapon.down = true;
				}
			}

		}
		else
		{
			wasLeftButtonPressed = true;
			//cout << "left click" << endl;
		}
	}
}

void Game::onMouseButtonUp(SDL_MouseButtonEvent event)
{
}

void Game::onMouseWheel(SDL_MouseWheelEvent event)
{
	mouse_speed *= event.y > 0 ? 1.1 : 0.9;
}

void Game::onResize(int width, int height)
{
    std::cout << "window resized: " << width << "," << height << std::endl;
	glViewport( 0,0, width, height );
	camera->aspect =  width / (float)height;
	window_width = width;
	window_height = height;
}

