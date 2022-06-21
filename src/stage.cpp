#include "stage.h"
#include "game.h"
#include "input.h"
#include <cmath>


#pragma region SUPLEMENTARY_FUNCTION
//Read scene file
void LoadSceneFile(const char* fileName, vector<EntityMesh*> *entities, vector<EntityMesh*> *entitiesWithColision)
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
				newEntity->model.scale(2.5f, 2.5f, 2.5f);
				newEntity->model.translate(0.0f, -0.02f, 0.0f);
			}
			bool roof = meshName == "roof_raised.obj" || meshName == "roof_flat.obj" || meshName == "door_frame.obj";
			if (!roof) {
				entitiesWithColision->push_back(newEntity);
			}

			entities->push_back(newEntity);
		}
	}
}
void LoadEnemiesInScene(const char* fileName, vector<EntityMesh*> *enemies)
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
			float meshScale = sceneParser.getfloat();

			string meshPath = "data/playermodels/" + meshName;
			string texPath = "data/playermodels/" + texName;
			newEntity->mesh = Mesh::Get(meshPath.c_str());
			if (!newEntity->mesh) newEntity->mesh = new Mesh();
			newEntity->texture = Texture::Get(texPath.c_str());
			if (!newEntity->texture) newEntity->texture = Texture::getBlackTexture();
			newEntity->name = meshName;
			newEntity->pos = MeshPosition;
			newEntity->scale = meshScale;
			enemies->push_back(newEntity);
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

	a_shader->setUniform("u_tex_tiling", 1.0f);

	a_shader->setUniform("u_model", model);
	//a_mesh->renderBounding(model);
	a_mesh->render(GL_TRIANGLES);

	//disable shader
	a_shader->disable();

	//if (Camera::current) a_mesh->renderBounding(model);


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
COL_RETURN CheckColision(Vector3 pos, Vector3 nexPos, EntityMesh* entity,float elapsed_time, float radius = 2.0f)
{

	COL_RETURN ret;
	Vector3 character_center = nexPos + Vector3(0, 0.35f, 0);
	if (radius == 0.75f) character_center = nexPos + Vector3(0, 1.225f, 0);

	Vector3 coll;
	Vector3 collnorm;

	if (!entity->mesh->testSphereCollision(entity->model, character_center, radius, coll, collnorm)) {
		ret.colision = false;
		ret.modifiedPosition = nexPos;
		return ret;
	}

	if(radius == 0.75f) cout << "Colision!" << endl;
	Vector3 push_away = normalize(coll - character_center) * elapsed_time;
	nexPos = (pos - push_away); //move to previous pos but a little bit further
	//cuidado con la Y, si nuestro juego es 2D la ponemos a 0
	nexPos.y = 0;
	
	ret.colision = true;
	ret.modifiedPosition = nexPos;
	return ret;
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
			RenderMesh(entities[i]->model, entities[i]->mesh, entities[i]->texture, shader, cam);
	}
	Matrix44 playerModel;
	playerModel.translate(player->pos.x, player->pos.y, player->pos.z);
	playerModel.rotate(player->yaw * DEG2RAD, Vector3(0, 1, 0));

	Matrix44 camModel = playerModel;
	camModel.rotate(player->pitch * DEG2RAD, Vector3(1, 0, 0));
	Vector3 eye = playerModel * Vector3(0.0f, 0.45f, 0.0f);
	if(Stage_ID == TABERN)
		eye = playerModel * Vector3(0.0f, 2.45f, 0.0f);
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
	if(Stage_ID != TABERN) RenderMesh(swordModel, weapon.entity->mesh, weapon.entity->texture, shader, cam);
	if (Stage_ID == MAP) {
		if (sphereArena) {
			Matrix44 T;
			T.setTranslation(0.0f, 0.005f * sin(Game::instance->time), 0.0f);
			sphereArena->model = sphereArena->model * T;
			RenderMesh(sphereArena->model, sphereArena->mesh, sphereArena->texture, shader, cam);
		}
		if (sphereTabern) {
			Matrix44 T;
			T.setTranslation(0.0f, 0.005f * sin(Game::instance->time), 0.0f);
			sphereTabern->model = sphereTabern->model * T;
			RenderMesh(sphereTabern->model, sphereTabern->mesh, sphereTabern->texture, shader, cam);
		}
	}
	RenderPlane(terrain->model, terrain->mesh, terrain->texture, terrain->shader, cam, tiling);
	if (Stage_ID == ARENA)
	{
		for (size_t i = 0; i < enemies.size(); i++)
		{
			Matrix44 enemyModel = Matrix44();
			enemyModel.translate(enemies[i]->pos.x, enemies[i]->pos.y, enemies[i]->pos.z);
			enemyModel.scale(enemies[i]->scale, enemies[i]->scale, enemies[i]->scale);
			//float ang = acos(clamp(enemyModel.frontVector().normalize().dot(to_target.normalize()), -1.0f, 1.0f));
			enemyModel.rotate(enemies[i]->yaw * DEG2RAD, Vector3(0, 1, 0));
			enemies[i]->model = enemyModel;
			RenderMesh(enemyModel, enemies[i]->mesh, enemies[i]->texture, shader, cam);
		}
		
	}
	if (Stage_ID == TABERN)
	{
		if (barTender) {
			Matrix44 npc_model = Matrix44();
			npc_model.translate(barTender->pos.x, barTender->pos.y, barTender->pos.z); 
			npc_model.scale(barTender->scale, barTender->scale, barTender->scale);
			npc_model.rotate(barTender->yaw * DEG2RAD, Vector3(0, 1, 0));
			barTender->model = npc_model;
			RenderMesh(npc_model, barTender->mesh, barTender->texture, shader, cam);
		}
	}

	drawCrosshair();
	Camera::current = cam;
}

void GameStage::update(float elapsed_time)
{
#pragma region SCENE_LOADER
	if (mapSwap)
	{
		
		entities.clear();
		entitiesColision.clear();
		switch (Stage_ID) {
		case MAP:
			if (entities.size() > 0) 
				entities.clear();
			if (entitiesColision.size() > 0)
				entities.clear();
			LoadSceneFile("data/MapJordiAlex.scene", &entities, &entitiesColision);
			if (previous_stage == ARENA) {
				player->pos = Vector3(0.69f, 0.0f, -9.82f);
				player->yaw = -3.34f;
			}
			else if (previous_stage == TABERN) {
				player->pos = Vector3(-7.60f, 0.0f, 3.58f);
				player->yaw = -1504.69f;
			}
			else player->pos = Vector3(30.12f,0.0f,16.88f);
			player->yaw = -249.8f;
			sphereArena = new EntityMesh();
			sphereTabern = new EntityMesh();
			sphereArena->mesh = Mesh::Get("data/arrow.obj");
			sphereArena->texture = Texture::Get("data/arrow.png");
			sphereArena->model = Matrix44();
			sphereArena->model.translate(0.57f, 0.25f, -12.66f);
			//sphereArena->model.scale(0.5f,0.5f,0.5f);
			sphereTabern->mesh = Mesh::Get("data/arrow.obj");
			sphereTabern->texture = Texture::Get("data/arrow.png");
			sphereTabern->model = Matrix44();
			sphereTabern->model.translate(-11.005f, 0.25f, 2.67f);
			//sphereTabern->model.scale(0.5f, 0.5f, 0.5f);
			Sleep(250);
			break;
		case ARENA:
			LoadSceneFile("data/ArenaJordiAlex.scene", &entities, &entitiesColision);
			LoadEnemiesInScene("data/enemies.scene", &enemies);
			player->pos = Vector3(23.52f,0.0f,-27.64f);
			player->yaw = -435.6f;
			Sleep(250);
			break;
		case TABERN:
			LoadSceneFile("data/TabernJordiAlex.scene", &entities, &entitiesColision);
			if (!barTender) {
				barTender = new EntityMesh();
				barTender->mesh = Mesh::Get("data/playermodels/Character7.obj");
				barTender->texture = Texture::Get("data/playermodels/Character.png");
				barTender->pos = Vector3(0.03f, 0.1f, 1.6f);
				barTender->scale = 0.25f;
				barTender->model = Matrix44();
			}
			player->pos = Vector3(0.01f,0.0f,-8.30f);
			player->yaw = -360.25f;
			//terrain->texture = Texture::Get("data/textures/arena.png");
			Sleep(250);
			break;

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
		COL_RETURN ret;
		
		if (CheckColision(player->pos, nexPos, sphereArena, elapsed_time, 1.5f).colision) {
			mapSwap = true;
			previous_stage = Stage_ID;
			Stage_ID = ARENA;
		}
		else if (CheckColision(player->pos, nexPos, sphereTabern, elapsed_time).colision) {
			mapSwap = true;
			previous_stage = Stage_ID;
			Stage_ID = TABERN;
		}
		else {
			for (size_t i = 0; i < entities.size(); i++)
			{
				ret = CheckColision(player->pos, nexPos, entities[i], elapsed_time, 1.0f);
				if (ret.colision) {
					nexPos = ret.modifiedPosition;
					break;
				}
			}
			player->pos = nexPos;
		}

	}
	else if (Stage_ID == TABERN)
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
		COL_RETURN ret;
		for (size_t i = 0; i < entitiesColision.size(); i++)
		{
			ret = CheckColision(player->pos, nexPos, entitiesColision[i], elapsed_time, 0.75f);
			if (ret.colision) {
				if (entitiesColision[i]->name._Equal("DOOR.OBJ")) {
				mapSwap = true;
					previous_stage = Stage_ID;
					Stage_ID = MAP;
				}
				else {
					nexPos = ret.modifiedPosition;
				}
				break;
			}
		}
		player->pos = nexPos;
	}
	else {
		if (isBattle)
		{
			weapon.movementMotion = false;
			if (currentEnemy->hp > 0) {
				if (currentEnemy->EmptyAttacks()) {
					//Wait until hit
					if (weapon.attacked) {
						currentEnemy->GenerateAttacks();
						currentEnemy->hp--;
						cout << "Enemy current health: " << currentEnemy->hp << endl;
						weapon.attacked = false;
					}
				}
				else {
					POSITION nextAttack = currentEnemy->GetNextAttack();
					//Animaciones enemigo
					if (nextAttack == UP) {
						//Animacion up
						cout << "UP" << endl;
					}
					else if (nextAttack == LEFT) {
						//Animacion left
						cout << "LEFT" << endl;
					}
					else{
						//Animacion right
						cout << "RIGHT" << endl;
					}

				}
			}
			else {
				currentEnemy->enemyEntity->destroy();
				for (size_t i = 0; i < enemies.size(); i++)
				{
					if (enemies[i] == currentEnemy->enemyEntity) {
						enemies.erase(enemies.begin() + i);
					}
				}
				currentEnemy = NULL;
				isBattle = false;
			}

			if (enemies.size() <= 0) {
				mapSwap = true;
				previous_stage = Stage_ID;
				Stage_ID = MAP;
				
			}
		}
		else {
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
		
				COL_RETURN ret;
				for (size_t i = 0; i < enemies.size(); i++)
				{
					ret = CheckColision(player->pos, nexPos, enemies[i], elapsed_time);
					if (ret.colision) {
						nexPos = ret.modifiedPosition;
						isBattle = true;
						currentEnemy = new EnemyAI(3, enemies[i], 2);
						currentEnemy->GenerateAttacks();
						enemies[i]->pos = Vector3(51.8f, 0.0f, -22.9f); //Center for enemy
						nexPos = Vector3(51.7f, 0.0f, -21.9f); //Center for player
						player->yaw = -535.0f; //Rotate player to enemy
						Sleep(300);
						break;
					}
				}
			player->pos = nexPos;
		}

	}
	if ((Input::mouse_state & SDL_BUTTON_LEFT)) //is left button pressed?
	{
		player->yaw += -Input::mouse_delta.x * 5.0f * elapsed_time;
		SDL_ShowCursor(false);
		Input::centerMouse();		
	}
#pragma endregion PLAYER_MOVEMENT

#pragma region CAMERA_MOVEMENT
	player->yaw += -Input::mouse_delta.x * 5.0f * elapsed_time;
	SDL_ShowCursor(false);
	if(!debug)
		Input::centerMouse();


#pragma endregion CAMERA_MOVEMENT

#pragma region WEAPON_MOVEMENT
	float weapon_speed = 100.0f;
	if (weapon.attack)
	{
		if (weapon.down)
			weapon.attackMotion -= 5.0f * elapsed_time * weapon_speed;
		else
			weapon.attackMotion += 5.0f * elapsed_time * weapon_speed;
		if (weapon.attackMotion <= -90.0f)
		{
			weapon.down = false;
		}
		if (weapon.attackMotion >= 0.0f)
		{
			weapon.attack = false;
			weapon.attackMotion = 0.0f;
		}
	}
	if (weapon.defence)
	{
		if (weapon.defType == LEFT)
		{
			//defence left behaviour
			if (weapon.defMotion)
				weapon.defenceMotion += 0.75f * elapsed_time * weapon_speed;
			else
				weapon.defenceMotion -= 0.75f * elapsed_time * weapon_speed;

			if (weapon.defenceMotion >= 10.0f)
			{
				weapon.defMotion = false;
			}

			if (weapon.defenceMotion <= 0.0f)
			{
				weapon.defence = false;
				weapon.defenceMotion = 0.0f;
			}

		}
		else if (weapon.defType == RIGHT)
		{
			//defence right behaviour
			if (weapon.defMotion)
				weapon.defenceMotion -= 0.5f * elapsed_time * weapon_speed;
			else
				weapon.defenceMotion += 0.5f * elapsed_time * weapon_speed;

			if (weapon.defenceMotion < -2.5f)
			{
				weapon.defMotion = false;
			}

			if (weapon.defenceMotion >= 0.0f)
			{
				weapon.defence = false;
				weapon.defenceMotion = 0.0f;
			}
		}
		else {
			//defence up behaviour
			
			//Weapon rotation
			if (weapon.defRotation)
				weapon.defenceRotation -= 7.5f * elapsed_time * weapon_speed;

			if (weapon.defenceRotation <= -90.0f)
				weapon.defRotation = false;
			if (!weapon.defRotation)
			{
				if (weapon.defMotionUp)
					weapon.defenceMotionUp += 1.25f * elapsed_time * weapon_speed;

				if (weapon.defenceMotionUp > 8.0f)
					weapon.defMotionUp = false;
				if (!weapon.defMotionUp)
				{
					weapon.defenceRotation += 7.5f * elapsed_time * weapon_speed;
					if (weapon.defenceRotation >= 0.0f)
					{
						if (weapon.defenceMotionUp > 0.0f)
							weapon.defenceMotionUp -= 1.25f * elapsed_time * weapon_speed;
						weapon.defenceRotation = 0.0f;

						if (weapon.defenceMotionUp <= 0.0f)
						{
							weapon.defenceMotionUp = 0.0f;
							weapon.defence = false;
						}
					}
				}
			}


		}
	}
	else
	{
		weapon.defType = NONE;
	}
#pragma endregion WEAPON_MOVEMENT
	if (!mapSwap) {
#pragma region ENEMY_BEHAVIOUR
		if (Stage_ID == ARENA) {
			//lookat of enemies
			for (size_t i = 0; i < enemies.size(); i++)
			{
				Vector3 to_target =  enemies[i]->pos - player->pos;
				to_target.normalize();
				Vector3 side = enemies[i]->model.rotateVector(Vector3(1, 0, 0)).normalize();
				float sideDot = side.dot(to_target);
				if (sideDot > 0.0f) {
					enemies[i]->yaw += 90.0f * elapsed_time;
				}
				else{
					enemies[i]->yaw -= 90.0f * elapsed_time;
				}
		

			}
		}
#pragma endregion ENEMY_BEHAVIOUR
	
	
#pragma region BARTENDER_BEHAVIOUR
		if (Stage_ID == TABERN) {
			Vector3 to_target = barTender->pos - player->pos;
			to_target.normalize();
			Vector3 side = barTender->model.rotateVector(Vector3(1, 0, 0)).normalize();
			float sideDot = side.dot(to_target);
			if (sideDot > 0.0f) {
				barTender->yaw += 90.0f * elapsed_time;
			}
			else {
				barTender->yaw -= 90.0f * elapsed_time;
			}
		}
#pragma enregion BARTENDER_BEHAVIOUR

	}
	if(!mapSwap) previous_stage = Stage_ID;
}

//GameOverStage

void GameOverStage::render()
{
}

void GameOverStage::update(float elapsed_time)
{
}