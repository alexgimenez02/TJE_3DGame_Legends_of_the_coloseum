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
#include <bass.h>
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

	//gameAudio = new Audio();
	//gameAudio = Audio::Get("");
	audios = get_all_audio_files(); 
	/*
	0, 1, 2 -> sword clash
	3 -> 7 -> damage soundEffect
	*/
	songs = get_all_song_files();
	/*
	0 -> Coliseum
	1 -> Intro+Controls
	2 -> Map
	3 -> Tabern
	*/
	//gameAudio->Play("data/audio/Coliseum OST.wav",0);
	if (BASS_Init(-1, 44100, 0, 0, NULL) == false) //-1 significa usar el por defecto del sistema operativo
	{
		//error abriendo la tarjeta de sonido...
		cout << "Error initializing output" << endl;
	}
	current_song = songs[1];
	currentSound = BASS_SampleLoad(false, current_song.c_str(), 0, 0, 3, 0);
	if (currentSound == 0) {
		cout << "Error audio not found!" << endl;
	}
	currentChannel = BASS_SampleGetChannel(currentSound, false);
	


	BASS_ChannelPlay(currentChannel, true);
	//current_song = songs[1];
	//PlayGameSound(current_song.c_str(), true);
	

	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");
	//create our camera
	camera = new Camera();
	camera->lookAt(Vector3(0.f,100.f, 100.f),Vector3(0.f,0.f,0.f), Vector3(0.f,1.f,0.f)); //position the camera and point to 0,0,0
	camera->setPerspective(70.f,window_width/(float)window_height,0.1f,10000.f); //set the projection, we want to be perspective


	//load one texture without using the Texture Manager (Texture::Get would use the manager)
	//LoadSceneFile("data/MapJordiAlex.scene");
	
	
	InitIntroStage();
	InitControlsStage();
	InitGameStage();
	InitGameOver();

	const char* x = "data/animations/Character";
	sanimation = getAnimation(x);

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
		if (current_song != songs[1]) {
			Audio::Stop(currentChannel);
			current_song = songs[1];
			currentSound = BASS_SampleLoad(false, current_song.c_str(), 0, 0, 3, 0);
			if (currentSound == 0) {
				cout << "Error audio not found!" << endl;
			}
			currentChannel = BASS_SampleGetChannel(currentSound, false);
			BASS_ChannelPlay(currentChannel, true);
		}
		SDL_ShowCursor(true);
		break;
	case CONTROLS:
		current_stage = controls;
		break;
	case GAME:
		current_stage = game_s;
		if (current_song == songs[1])
		{
			Audio::Stop(currentChannel);
			current_song = songs[2];
			currentSound = BASS_SampleLoad(false, current_song.c_str(), 0, 0, 3, 0);
			if (currentSound == 0) {
				cout << "Error audio not found!" << endl;
			}
			currentChannel = BASS_SampleGetChannel(currentSound, false);
			BASS_ChannelPlay(currentChannel, true);
		}
		else {
			bool changeSound = false;
			switch (game_s->Stage_ID) {
			case MAP:
				if (current_song != songs[2]) {
					current_song = songs[2];
					changeSound = true;
				}
				break;
			case ARENA:
				if (current_song != songs[0]) {
					current_song = songs[0];
					changeSound = true;
				}
				break;
			case TABERN:
				if (current_song != songs[3]) {
					current_song = songs[3];
					changeSound = true;
				}
				break;
			
			}
			if (changeSound) {
				Audio::Stop(currentChannel);
				currentSound = BASS_SampleLoad(false, current_song.c_str(), 0, 0, 3, 0);
				if (currentSound == 0) {
					cout << "Error audio not found!" << endl;
				}
				currentChannel = BASS_SampleGetChannel(currentSound, false);
				BASS_ChannelPlay(currentChannel, true);
			}
			break;
		}
		
		break;
	case GAMEOVER:
		current_stage = gameOver;
		/*
		Game over song
		*/
		break;
	}
	BASS_ChannelSetAttribute(currentChannel, BASS_ATTRIB_VOL, volume);
}

//Keyboard event handler (sync input)
void Game::onKeyDown( SDL_KeyboardEvent event )
{
	switch(event.keysym.sym)
	{
		case SDLK_ESCAPE:  
			if (current_stage == game_s) {
				GameStage* stg = (GameStage*)current_stage;
				if (stg->interaction) {
					stg->menu = !stg->interaction;
					stg->interaction = stg->menu;
					stg->nextText = 0;
				}
				else {
					stg->menu = !stg->menu;
				}
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
						stg->parry = 0.0f;
						stg->weapon.defence = true; 
						stg->weapon.defType = RIGHT;
						stg->weapon.defMotion = true;
						cout << "Right defence" << endl;
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
						stg->parry = 0.0f;
						stg->weapon.defence = true;
						stg->weapon.defType = LEFT;
						stg->weapon.defMotion = true;
						cout << "Left defence" << endl;
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
						stg->parry = 0.0f;
						stg->weapon.defence = true;
						stg->weapon.defType = UP;
						stg->weapon.defMotion = true;
						stg->weapon.defMotionUp = true;
						stg->weapon.defRotation = true;
						cout << "Up defence" << endl;
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
				else {
					wasLeftButtonPressed = true;
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

void Game::InitIntroStage()
{
	//Intro stage init
	intro = new IntroStage();

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
	intro->terrain->texture = Texture::Get("data/sand.tga");
	intro->terrain->shader = shader;
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
}

void Game::InitControlsStage()
{
	//Controls stage init
	controls = new ControlsStage();

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
}



void Game::InitGameStage() {
	//Game Stage Init
	game_s = new GameStage();

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
		1,
		0.0f,
		0.1f
	};
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
}

void Game::InitGameOver()
{
	gameOver = new GameOverStage();
}

