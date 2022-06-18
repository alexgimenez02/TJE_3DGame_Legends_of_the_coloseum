#include "stage.h"
#include "game.h"
#include "input.h"
#include <cmath>


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

//IntroStage

void IntroStage::render()
{
	//GUI RENDER
	
	glDisable(GL_DEPTH_BUFFER);

	//el vertice lo pasamos tal cual
	

	//un ejemplo de quad que llenaria toda la pantalla (ya que va de -1 a +1)

	Mesh quad;

	//tres vertices del primer triangulo
	quad.vertices.push_back( Vector3(-1,1,0) );
	quad.uvs.push_back( Vector2(0,1) );
	quad.vertices.push_back( Vector3(-1,-1,0) );
	quad.uvs.push_back( Vector2(0,0) );
	quad.vertices.push_back( Vector3(1,-1,0) );
	quad.uvs.push_back( Vector2(1,0) );

	//tres vértices del segundo triángulo
	quad.vertices.push_back( Vector3(-1,1,0) );
	quad.uvs.push_back( Vector2(0,1) );
	quad.vertices.push_back( Vector3(1,-1,0) );
	quad.uvs.push_back( Vector2(1,0) );
	quad.vertices.push_back( Vector3(1,1,0) );
	quad.uvs.push_back( Vector2(1,1) );


	//para pintar quad
	//pasar info al shader
	//...
	//hacer draw call
	quad.render( GL_TRIANGLES );
	
}

void IntroStage::update(float elapsed_time)
{
	//OPTION SELECTION
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
	//set the camera as default
	//Parentar model --> Offset respecte player que defineix on esta l'arma
	Matrix44 swordModel;
	Vector3 swordOffset = weapon.weaponOffset;
	swordModel.setTranslation(swordOffset.x, swordOffset.y, swordOffset.z);
	swordModel.rotate(-90.0f * DEG2RAD, Vector3(0, 1, 0));
	swordModel.scale(1 / 25.0f, 1 / 25.0f, 1 / 25.0f);
	swordModel = swordModel * playerModel;
	if (weapon.defType!= NONE)
	{
		if (weapon.defType== UP)
		{
			//motion defence up
			swordModel.translate(0.0f, weapon.defenceMotionUp, weapon.defenceMotion);
			swordModel.rotate(weapon.defenceRotation * DEG2RAD, Vector3(1, 0, 0));
		}
		else
		{
			swordModel.translate(0.0f, 0.0f, weapon.defenceMotion);
		}
	}
	if (weapon.movementMotion)
	{
		Matrix44 T;
		T.setTranslation(0.0f, 0.025f * sin(Game::instance->time) + 0.05f, 0.0f);
		swordModel = swordModel * T;
	}
	swordModel.rotate(weapon.attackMotion * DEG2RAD, Vector3(0, 0, 1));
	RenderMesh(swordModel, weapon.entity->mesh, weapon.entity->texture, shader, cam);

	RenderPlane(terrain->model, terrain->mesh, terrain->texture, terrain->shader, cam, tiling);

	//drawCrosshair();
	Camera::current = cam;
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
		weapon.movementMotion = playerVel.x != 0 || playerVel.y != 0 || playerVel.z != 0;

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
		player->yaw += -Input::mouse_delta.x * 5.0f * elapsed_time;
		SDL_ShowCursor(false);
		Input::centerMouse();		
	}
#pragma endregion PLAYER_MOVEMENT

#pragma region WEAPON_MOVEMENT



#pragma endregion WEAPON_MOVEMENT
}

//GameOverStage

void GameOverStage::render()
{
}

void GameOverStage::update(float elapsed_time)
{
}