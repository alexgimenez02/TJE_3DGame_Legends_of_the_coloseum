#include "stage.h"
#include "game.h"
#include "input.h"

/*
#pragma region SUPLEMENTARY_FUNCTION
//Read scene file
void LoadSceneFile(const char* fileName, vector<EntityMesh*> *entities)
{
	TextParser sceneParser = TextParser();
	if (!sceneParser.create(fileName)) return;
	cout << "File loaded correctly!" << endl;
	sceneParser.seek("BITS");
	while (sceneParser.eof() == 0)
	{
		if (sceneParser.getword() == string::basic_string("MESH"))
		{
			EntityMesh* newEntity = new EntityMesh();
			string meshName = sceneParser.getword();
			string texName = sceneParser.getword();
			Vector3 MeshPosition = Vector3();
			MeshPosition.x = sceneParser.getfloat();
			MeshPosition.y = sceneParser.getfloat();
			MeshPosition.z = sceneParser.getfloat();
			newEntity->model._11 = sceneParser.getfloat();
			newEntity->model._12 = sceneParser.getfloat();
			newEntity->model._13 = sceneParser.getfloat();
			newEntity->model._14 = sceneParser.getfloat();
			newEntity->model._21 = sceneParser.getfloat();
			newEntity->model._22 = sceneParser.getfloat();
			newEntity->model._23 = sceneParser.getfloat();
			newEntity->model._24 = sceneParser.getfloat();
			newEntity->model._31 = sceneParser.getfloat();
			newEntity->model._32 = sceneParser.getfloat();
			newEntity->model._33 = sceneParser.getfloat();
			newEntity->model._34 = sceneParser.getfloat();
			newEntity->model._41 = sceneParser.getfloat();
			newEntity->model._42 = sceneParser.getfloat();
			newEntity->model._43 = sceneParser.getfloat();
			newEntity->model._44 = sceneParser.getfloat();
			int layer, bits;
			layer = sceneParser.getint();
			bits = sceneParser.getint();

			string meshPath = "data/props/" + meshName;
			string texPath = "data/textures/" + texName;
			newEntity->mesh = Mesh::Get(meshPath.c_str());
			if (!newEntity->mesh) newEntity->mesh = new Mesh();
			newEntity->texture = Texture::Get(texPath.c_str());
			if (!newEntity->texture) newEntity->texture = Texture::getBlackTexture();
			newEntity->name = meshName;
			newEntity->pos = MeshPosition;
			if (fileName == "data/ArenaJordiAlex.scene")
			{
				cout << "Arena" << endl;
				newEntity->model.scale(30.0f, 30.0f, 30.0f);
				newEntity->model.translate(0.0f, -0.02f, 0.0f);
			}
			entities->push_back(newEntity);
		}
	}
}
//Render specified mesh
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

	if (Camera::current)	a_mesh->renderBounding(model);


}
//
void RenderPlane(Matrix44 model, Mesh* a_mesh, Texture* tex, Shader* a_shader, Camera* cam, float tiling) {
	if (a_shader)
	{
		//enable shader
		a_shader->enable();

		//upload uniforms
		a_shader->setUniform("u_color", Vector4(1, 1, 1, 1));
		a_shader->setUniform("u_viewprojection", cam->viewprojection_matrix);
		a_shader->setUniform("u_texture", tex, 0);
		a_shader->setUniform("u_time", time);
		a_shader->setUniform("u_tex_tiling", tiling);

		Matrix44 m;


		Vector3 size = a_mesh->box.halfsize * 2;
		//m.setTranslation(size.x, 0.0f, size.z);
		a_shader->setUniform("u_model", m);
		//do the draw call
		a_mesh->render(GL_TRIANGLES);


		//disable shader
		a_shader->disable();
	}
}
#pragma region COLISION
bool CheckColision(Vector3 pos, Vector3 *nexPos, vector<EntityMesh*> entities,float elapsed_time)
{
	Vector3 character_center = *nexPos + Vector3(0, 0.15f, 0);

	for (size_t i = 0; i < entities.size(); i++)
	{
		EntityMesh* entity = entities[i];

		Vector3 coll;
		Vector3 collnorm;

		if (!entity->mesh->testSphereCollision(entity->model, character_center, 2.0f, coll, collnorm)) false;

		Vector3 push_away = normalize(coll - character_center) * elapsed_time;
		nexPos = &(pos - push_away); //move to previous pos but a little bit further
		//cuidado con la Y, si nuestro juego es 2D la ponemos a 0
		nexPos->y = 0;
	}
	return true;
}
#pragma endregion COLISION

#pragma region CROSSHAIR
void drawCrosshair()
{
	float scale = 5;
	int half_width = Game::instance->window_width / 2;
	int half_heigth = Game::instance->window_height / 2;
	drawText(half_width - 14, half_heigth - 5, "+", Vector3(0, 0, 0), scale);
}
#pragma endregion CROSSHAIR	
#pragma endregion
*/
//IntroStage

void IntroStage::render()
{
}

void IntroStage::update(float elapsed_time)
{
}


//ControlsStage

void ControlsStage::render()
{
}

void ControlsStage::update(float elapsed_time)
{
}



//GameStage

void GameStage::render()
{
	//set the clear color (the background color)
	glClearColor(0.0, 0.0, 0.0, 1.0);

	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//set flags
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	Camera* cam = Game::instance->camera;
	glDisable(GL_DEPTH_TEST);
	sky->render();
	glEnable(GL_DEPTH_TEST);
	if (!entities.empty())
	{
		for (size_t i = 0; i < entities.size(); i++)
			entities[i]->render();
	}
	Matrix44 playerModel;
	playerModel.translate(player->pos.x, player->pos.y, player->pos.z);
	playerModel.rotate(player->yaw * DEG2RAD, Vector3(0, 1, 0));

	Matrix44 camModel = playerModel;
	camModel.rotate(player->pitch * DEG2RAD, Vector3(1, 0, 0));
	Vector3 eye = playerModel * Vector3(0.1f, 0.45f, 2.25f);
	Vector3 center = cam->center;
	if (!yAxisCam)
		center = eye + camModel.rotateVector(Vector3(0, 0, 1));
	Vector3 up = camModel.rotateVector(Vector3(0, 1, 0));
	cam->enable();
	cam->lookAt(eye, center, up);

	//RenderPlane(terrain->model, terrain->mesh, terrain->texture, terrain->shader, cam, tiling);

	//drawCrosshair();
}

void GameStage::update(float elapsed_time)
{
#pragma region SCENE_LOADER
	if (mapSwap)
	{
		entities.clear();
		if (Stage_ID == MAP)
		{
			//LoadSceneFile("data/MapJordiAlex.scene", &entities);
		}
		else if (Stage_ID == ARENA)
		{
			//LoadSceneFile("data/ArenaJordiAlex.scene", &entities);
		}
		else if (Stage_ID == TABERN)
		{
			//LoadSceneFile("data/TabernJordiAlex.scene", &entities);
		}
		mapSwap = false;
	}
#pragma endregion SCENE_LOADER
#pragma region PLAYER_MOVEMENT
	float playerSpeed = 5.0f * elapsed_time;
	if (Stage_ID == MAP)
	{
		Matrix44 playerRotation;
		playerRotation.rotate(player->yaw * DEG2RAD, Vector3(0, 1, 0));
		Vector3 playerVel;
		Vector3 forward = playerRotation.rotateVector(Vector3(0, 0, -1));
		Vector3 right = playerRotation.rotateVector(Vector3(1, 0, 0));
		if (Input::isKeyPressed(SDL_SCANCODE_LSHIFT)) playerSpeed *= 2.0f;
		if (Input::isKeyPressed(SDL_SCANCODE_W)) playerVel = playerVel - (forward * playerSpeed);
		if (Input::isKeyPressed(SDL_SCANCODE_S)) playerVel = playerVel + (forward * playerSpeed);
		if (Input::isKeyPressed(SDL_SCANCODE_A)) playerVel = playerVel + (right * playerSpeed);
		if (Input::isKeyPressed(SDL_SCANCODE_D)) playerVel = playerVel - (right * playerSpeed);

		Vector3 nexPos = player->pos + playerVel;

		Vector3 character_center = nexPos + Vector3(0, 0.15f, 0);

		for (size_t i = 0; i < entities.size(); i++)
		{
			//if (CheckColision(player->pos, &nexPos, entities, elapsed_time)) break;
		}
		player->pos = nexPos;
	}
	if ((Input::mouse_state & SDL_BUTTON_LEFT)) //is left button pressed?
	{
		
		player->pitch += Input::mouse_delta.y * 5.0f * elapsed_time;
#pragma region CAMERA_Y_BOUNDARIES
		{
			player->pitch = player->pitch < -90 ? -90 : player->pitch;
			player->pitch = player->pitch > 90 ? 90 : player->pitch;
		}
		player->yaw += -Input::mouse_delta.x * 5.0f * elapsed_time;
		SDL_ShowCursor(false);
		Input::centerMouse();		
	}
#pragma endregion CAMERA_Y_BOUNDARIES
#pragma region PLAYER_MOVEMENT
}

//GameOverStage

void GameOverStage::render()
{
}

void GameOverStage::update(float elapsed_time)
{
}