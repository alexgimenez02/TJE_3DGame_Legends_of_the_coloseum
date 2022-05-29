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
bool cameraLocked = false, yAxisCam = false, checkCol = false, editorMode = false;


bool meshSwap = false;
int currMeshIdx = 0;
vector<string> meshnames, texnames;
string currentMesh = "data/editor/barn.obj", currentTex = "data/editor/materials.tga";

EntityMesh player;
EntityMesh preview;
EntityMap terrain;
EntityMap sky;

Mesh* groundMesh;
Texture* groundTex;
vector<EntityMesh*> meshes;
Game* Game::instance = NULL;
EntityMesh* SelectedEntity;


vector<string> get_all_files_names_within_folder(bool isMesh)
{
	vector<string> names;
	LPCWSTR search_path = L" ";
	if (isMesh) search_path = L"data/editor/*.obj";
	else
	{
		LPCWSTR paths[] = { L"data/editor/*.tga" , L"data/editor/*.png", L"data/editor/*.jpg" };
		for (size_t i = 0; i < 3; i++)
		{
			search_path = paths[i];
			WIN32_FIND_DATA fd;
			HANDLE hFind = ::FindFirstFile(search_path, &fd);
			if (hFind != INVALID_HANDLE_VALUE) {
				do {
					// read all (real) files in current folder
					// , delete '!' read other 2 default folder . and ..
					if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
						wstring recieve = fd.cFileName;
						string toAdd(recieve.begin(), recieve.end());
						names.push_back("data/editor/" + toAdd);
					}
				} while (::FindNextFile(hFind, &fd));
				::FindClose(hFind);
			}

		}
		return names;
	}
	if (search_path == L" ") return names;
	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFile(search_path, &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			// read all (real) files in current folder
			// , delete '!' read other 2 default folder . and ..
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				wstring recieve = fd.cFileName;
				string toAdd(recieve.begin(), recieve.end());
				names.push_back("data/editor/"+toAdd);
			}
		} while (::FindNextFile(hFind, &fd));
		::FindClose(hFind);
	}
	return names;
}
void LoadSceneFile(const char* fileName)
{
	TextParser sceneParser = TextParser();
	if (!sceneParser.create(fileName)) return;
	cout << "File loaded correctly!" << endl;
	sceneParser.seek("INFO");
	while (sceneParser.eof() == 0)
	{
		if (sceneParser.getword() == string::basic_string("MESH"))
		{
			string meshName = sceneParser.getword();
			string texName = sceneParser.getword();
			Vector3 MeshPosition = Vector3(), EulerRotation = Vector3(), ScaleVector = Vector3();
			MeshPosition.x = sceneParser.getfloat();
			MeshPosition.y = sceneParser.getfloat();
			MeshPosition.z = sceneParser.getfloat();
			EulerRotation.x = sceneParser.getfloat();
			EulerRotation.y = sceneParser.getfloat();
			EulerRotation.z = sceneParser.getfloat();
			ScaleVector.x = sceneParser.getfloat();
			ScaleVector.y = sceneParser.getfloat();
			ScaleVector.z = sceneParser.getfloat();
			int layer, bits;
			layer = sceneParser.getint();
			bits = sceneParser.getint();


			string meshPath = "data/props/" + meshName;
			string texPath = "data/textures/" + texName;
			EntityMesh newEntity = EntityMesh();
			newEntity.mesh = Mesh::Get(meshPath.c_str());
			newEntity.texture = Texture::Get(texPath.c_str());
			newEntity.name = meshName;
			newEntity.pos = MeshPosition;
			newEntity.model.translate(newEntity.pos.x, newEntity.pos.y, newEntity.pos.z);
			newEntity.model.rotateVector(EulerRotation);
			newEntity.model.scale(ScaleVector.x, ScaleVector.y, ScaleVector.z);
			meshes.push_back(&newEntity);
		}
		//sceneParser.getword();
	}

}

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

	//create our camera
	camera = new Camera();
	camera->lookAt(Vector3(0.f,100.f, 100.f),Vector3(0.f,0.f,0.f), Vector3(0.f,1.f,0.f)); //position the camera and point to 0,0,0
	camera->setPerspective(70.f,window_width/(float)window_height,0.1f,10000.f); //set the projection, we want to be perspective


	//load one texture without using the Texture Manager (Texture::Get would use the manager)
	//Goblin
	LoadSceneFile("data/MapJordiAlex.scene");
	player.texture = new Texture();
	player.texture->load("data/playermodels/Character.png");
	player.name = "Player";
	//Terrain
	terrain.texture = new Texture();
	terrain.texture->load("data/terrain.tga");

	sky.texture = new Texture();
	sky.texture->load("data/cielo.tga");

	//Goblins
	tex = new Texture();
	tex->load("data/terrain.tga");
	// example of loading Mesh from Mesh Manager
	player.mesh = Mesh::Get("data/playermodels/Character1.obj");
	terrain.mesh = Mesh::Get("data/terrain.ASE");
	sky.mesh = Mesh::Get("data/cielo.ASE");
	mesh = Mesh::Get("data/editor/minihouse.obj");
	player.mesh->name = player.name;

	groundMesh = new Mesh();
	groundMesh->createPlane(1000);
	groundTex = Texture::Get("data/grass.tga");
	
	// example of shader loading using the shaders manager
	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");

	player.shader = shader;
	terrain.shader = shader;
	sky.shader = shader;

	meshnames = get_all_files_names_within_folder(true);
	texnames = get_all_files_names_within_folder(false);
	preview.mesh = Mesh::Get(currentMesh.c_str());
	preview.texture = Texture::Get(currentTex.c_str());

	//hide the cursor

	//goblin.mesh->createCollisionModel();
	SDL_ShowCursor(!mouse_locked); //hide or show the mouse
}


void RenderMesh(Matrix44 model, Mesh* a_mesh, Texture* tex, Shader* a_shader, Camera* cam) {
	//assert(a_mesh != null, "mesh in renderMesh was null");
	if (!a_shader) return;

	//enable shader
	a_shader->enable();
	//upload uniforms
	a_shader->setUniform("u_color", Vector4(1, 1, 1, 1));
	a_shader->setUniform("u_viewprojection", cam->viewprojection_matrix);
	if (tex != NULL) {
		a_shader->setUniform("u_texture", tex, 0);
	}
	
	a_shader->setUniform("u_time", time);

	//a_shader->setUniform("u_tex_tiling", 1.0f);

	a_shader->setUniform("u_model", model);
	//a_mesh->renderBounding(model);
	a_mesh->render(GL_TRIANGLES);

	//disable shader
	a_shader->disable();

	if(Camera::current)	a_mesh->renderBounding(model);

	
}
void RenderPlane(float tiling){
	if (shader)
	{
		//enable shader
		shader->enable();

		//upload uniforms
		shader->setUniform("u_color", Vector4(1, 1, 1, 1));
		shader->setUniform("u_viewprojection", Game::instance->camera->viewprojection_matrix);
		shader->setUniform("u_texture", groundTex, 0);
		shader->setUniform("u_time", time);
		shader->setUniform("u_tex_tiling", tiling);

		Matrix44 m;
		for (size_t i = 0; i < 20; i++)
		{
			for (size_t j = 0; j < 20; j++)
			{

				Vector3 size = groundMesh->box.halfsize * 2;
				m.setTranslation(size.x * i, 0.0f, size.z * j);
				shader->setUniform("u_model", m);
				//do the draw call
				groundMesh->render(GL_TRIANGLES);
				
			}

		}

		//disable shader
		shader->disable();
	}
}
void drawPreview(Matrix44 model, Mesh* a_mesh, Texture* tex, Shader* a_shader, Camera* cam)
{
	shader->enable();
	float scale = 0.5;
	Matrix44 projection_matrix;
	projection_matrix.ortho(0, Game::instance->window_width / scale, Game::instance->window_height / scale, 0, -1, 1);
	
	glDisable(GL_CULL_FACE);

	glLoadMatrixf(projection_matrix.m);

	a_mesh->render(GL_TRIANGLES);

	
	glEnable(GL_CULL_FACE);
	shader->disable();
}
//what to do when the image has to be draw
void Game::render(void)
{
	//set the clear color (the background color)
	glClearColor(0.0, 0.0, 0.0, 1.0);

	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//set flags
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
   
	//create model matrix for cube
	//m.rotate(angle*DEG2RAD, Vector3(0, 1, 0));
	//m.scale(50.0f, 50.0f, 50.0f);
	Camera* cam = Game::instance->camera;
	glDisable(GL_DEPTH_TEST);
	if (editorMode)
	{
		/*Matrix44 model;
		RenderMesh(model, Mesh::Get(currentMesh.c_str()), Texture::Get(currentTex.c_str()), shader, cam);
		*/
	}
	sky.render();
	glEnable(GL_DEPTH_TEST);
	if (!meshes.empty())
	{
		for (size_t i = 0; i < meshes.size(); i++)
		{
			RenderMesh(meshes[i]->model, meshes[i]->mesh, meshes[i]->texture, shader, cam);
		}
	}

	Matrix44 playerModel;
	playerModel.translate(player.pos.x, player.pos.y, player.pos.z);
	playerModel.rotate(player.yaw * DEG2RAD, Vector3(0, 1, 0));
	
	//set the camera as default
	if (cameraLocked) {
		Matrix44 camModel = playerModel;
		camModel.rotate(player.pitch * DEG2RAD, Vector3(1, 0, 0));
		Vector3 eye = playerModel * Vector3(0.1f, 7.0f, 2.25f);
		Vector3 center = camera->center;
		if(!yAxisCam)
			center = eye + camModel.rotateVector(Vector3(0,0,1));
		Vector3 up = camModel.rotateVector(Vector3(0,1,0));
		camera->enable();
		camera->lookAt(eye, center, up);
		
	}
	RenderMesh(playerModel, player.mesh, player.texture, shader, cam);	
	
	RenderPlane(60.0f);

	//Draw the floor grid
	//drawGrid();

	//render the FPS, Draw Calls, etc
	drawText(2, 2, getGPUStats(), Vector3(1, 1, 1), 2);
	//swap between front buffer and back buffer
	
	
	//QUITAR DE AQUI Y METER EN OTRO LADO IMPORTANTEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE
	float scale = 5;
	int half_width = window_width / 2;
	int half_heigth = window_height / 2;
	drawText(half_width - 14, half_heigth - 5, "+", Vector3(0, 0, 0), scale);
	//--------------------------------------------------------------------------------------
	
	Camera::current = cam;
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
	float speed = seconds_elapsed * mouse_speed; //the speed is defined by the seconds_elapsed so it goes constant

	//example
	angle += (float)seconds_elapsed * 10.0f;

	//mouse input to rotate the cam
	if ((Input::mouse_state & SDL_BUTTON_LEFT) || mouse_locked ) //is left button pressed?
	{
		if (cameraLocked)
		{
			
			
			player.pitch += Input::mouse_delta.y * 5.0f * elapsed_time;
			//Check limits
			{
				player.pitch = player.pitch < -90 ? -90 : player.pitch;
				player.pitch = player.pitch > 90 ? 90 : player.pitch;
			}
			player.yaw += -Input::mouse_delta.x * 5.0f * elapsed_time;
			SDL_ShowCursor(false);
			Input::centerMouse();
			//Mostrar crosshair
			
		}
		else {
			camera->rotate(Input::mouse_delta.x * 0.005f, Vector3(0.0f,-1.0f,0.0f));
			camera->rotate(Input::mouse_delta.y * 0.005f, camera->getLocalVector( Vector3(-1.0f,0.0f,0.0f)));
		}
	}
	if (editorMode)
	{
		if (Input::wasKeyPressed(SDL_SCANCODE_LEFT))
		{
			currMeshIdx--;
			meshSwap = true;
		}
		if (Input::wasKeyPressed(SDL_SCANCODE_RIGHT))
		{
			currMeshIdx++;
			meshSwap = true;
		}
		if (currMeshIdx < 0) currMeshIdx = meshnames.size() - 1;
		if (currMeshIdx > meshnames.size() - 1) currMeshIdx = 0;

		if (meshSwap)
		{
			cout << "Current mesh selected: " << meshnames[currMeshIdx] << endl;
			
			meshSwap = false;
			currentMesh = meshnames[currMeshIdx];
			string delimiter = ".";
			for (size_t i = 0; i < texnames.size(); i++)
			{
				if (texnames[i] == currentMesh.substr(0, currentMesh.find(delimiter)) + ".tga" || texnames[i] == currentMesh.substr(0, currentMesh.find(delimiter)) + ".png" || texnames[i] == currentMesh.substr(0, currentMesh.find(delimiter)) + ".jpg")
				{
					currentTex = texnames[i];
					break;
				}
				if (i == texnames.size() - 1)
				{
					currentTex = "data/editor/materials.tga";
				}
			}
			cout << "Current texture selected: " << currentTex << endl;
		}
	}
	//async input to move the camera around
	else {
		if (Input::wasKeyPressed(SDL_SCANCODE_TAB)) cameraLocked = !cameraLocked;
	}
	if (cameraLocked) {
		float playerSpeed = 25.0f * elapsed_time;
		//float playerSpeed = 1.5f * elapsed_time;
		//float rotSpeed = 500.0f * elapsed_time;
		Matrix44 playerRotation;
		playerRotation.rotate(player.yaw * DEG2RAD, Vector3(0,1,0));
		Vector3 playerVel;
		Vector3 forward = playerRotation.rotateVector(Vector3(0,0,-1));
		Vector3 right = playerRotation.rotateVector(Vector3(1, 0, 0));
		//player.pos = ply.model.getTranslation();
		if (Input::isKeyPressed(SDL_SCANCODE_LSHIFT)) playerSpeed *= 2.0f;
		if (Input::isKeyPressed(SDL_SCANCODE_W)) playerVel = playerVel - (forward * playerSpeed);
		if (Input::isKeyPressed(SDL_SCANCODE_S)) playerVel = playerVel + (forward * playerSpeed);
		if (Input::isKeyPressed(SDL_SCANCODE_A)) playerVel = playerVel + (right * playerSpeed);
		if (Input::isKeyPressed(SDL_SCANCODE_D)) playerVel = playerVel - (right * playerSpeed);
		
		
		//calculamos el centro de la esfera de colisión del player elevandola hasta la cintura
		Vector3 nexPos = player.pos + playerVel;

		Vector3 character_center = nexPos + Vector3(0, 4, 0);

		for (size_t i = 0; i < meshes.size(); i++)
		{
			EntityMesh* entity = meshes[i];
			
			Vector3 coll;
			Vector3 collnorm;

			if (!entity->mesh->testSphereCollision(entity->model, character_center, 3.25f, coll, collnorm)) continue;

			Vector3 push_away = normalize(coll - character_center) * elapsed_time;
			nexPos = player.pos - push_away; //move to previous pos but a little bit further
			//cuidado con la Y, si nuestro juego es 2D la ponemos a 0
			nexPos.y = 0;
		}
		player.pos = nexPos;

		
	}
	else {

		if (Input::isKeyPressed(SDL_SCANCODE_LSHIFT)) speed *= 10; //move faster with left shift
		if (Input::isKeyPressed(SDL_SCANCODE_W)) camera->move(Vector3(0.0f, 0.0f, 1.0f) * speed);
		if (Input::isKeyPressed(SDL_SCANCODE_S)) camera->move(Vector3(0.0f, 0.0f, -1.0f) * speed);
		if (Input::isKeyPressed(SDL_SCANCODE_A)) camera->move(Vector3(1.0f, 0.0f, 0.0f) * speed);
		if (Input::isKeyPressed(SDL_SCANCODE_D)) camera->move(Vector3(-1.0f, 0.0f, 0.0f) * speed);


	}

	//to navigate with the mouse fixed in the middle
	if (mouse_locked)
		Input::centerMouse();
}

//Keyboard event handler (sync input)
void Game::onKeyDown( SDL_KeyboardEvent event )
{
	switch(event.keysym.sym)
	{
		case SDLK_ESCAPE: must_exit = true; break; //ESC key, kill the app
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
		case SDLK_F5:
			cout << "Static meshes in scene" << endl;
			for (size_t i = 0; i < meshes.size(); i++)
			{
				cout << i << ") Mesh: " << meshes[i]->name.c_str() << endl;
			}
			break;
		case SDLK_KP_PLUS: RotateSelected(10.0f); break;
		case SDLK_KP_MINUS: RotateSelected(-10.0f); break;
		
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

