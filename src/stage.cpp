#include "stage.h"
#include "game.h"
#include "input.h"
#include <iostream>
#include <fstream>
#include <cmath>


GLenum error;
float rotationSpeedIntro  = 0.55f;
float posx, posy;
string text = "Left_arrow";

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
void RenderGUI(float x, float y, float w, float h, Shader* a_shader, Texture* tex, Vector4 tex_range, Vector4 color = Vector4(1, 1, 1, 1), bool flipUV = false) {
	
	int windowWidth = Game::instance->window_width;
	int windowHeight = Game::instance->window_height;
	
	Mesh quad;
	quad.createQuad(x, y, w, h, flipUV);

	Camera cam2D;
	cam2D.setOrthographic(0, windowWidth, windowHeight, 0, -1, 1);

	if (!a_shader)return;
	a_shader->enable();

	a_shader->setUniform("u_color", color);
	a_shader->setUniform("u_viewprojection", cam2D.viewprojection_matrix);

	if(tex != NULL) a_shader->setUniform("u_texture", tex, 0);

	a_shader->setUniform("u_time", time);
	a_shader->setUniform("u_tex_range", tex_range);	
	a_shader->setUniform("u_model", Matrix44());
	quad.render(GL_TRIANGLES);


	a_shader->disable();
	


}
bool RenderButton(float x, float y, float w, float h, Shader* a_shader, Texture* tex,Vector4 tex_range, Texture* text_hover = NULL, bool flipUV = false)
{
	Vector2 mouse = Input::mouse_position;
	float halfWidth = w * 0.5f;
	float halfHeight = h * 0.5f;
	float min_x = x - halfWidth;
	float max_x = x + halfWidth;
	float min_y = y - halfHeight;
	float max_y = y + halfHeight;


	bool hover = mouse.x >= min_x && mouse.x <= max_x && mouse.y >= min_y && mouse.y <= max_y;
	if (text_hover != NULL) {
		Texture* texture = hover ? text_hover : tex;
		RenderGUI(x, y, w, h, a_shader, texture, tex_range, Vector4(1,1,1,1), flipUV);
	}
	else {
		Vector4 color = hover ? Vector4(1, 1, 1, 1) : Vector4(1, 1, 1, 0.7f);
		RenderGUI(x, y, w, h, a_shader, tex, tex_range, color, flipUV);
	}

	bool pressed = Game::instance->wasLeftButtonPressed && hover;
	return pressed;
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
#pragma region COMPLEMENTARY_ICON_FUNCTIONS
void IntroStage::reloadIcons()
{
	icons.clear();
	positions.clear();
	icons = get_all_files_names_within_icons();
	for (size_t i = 0; i < icons.size(); i++)
	{
		positions.push_back(readPosition(icons[i].c_str()));
	}
}
#pragma endregion COMPLEMENTARY_ICON_FUNCTIONS

void IntroStage::render()
{
	glDisable(GL_DEPTH_TEST);
	sky->render();
	glEnable(GL_DEPTH_TEST);
	Matrix44 planeMatrix = Matrix44();
	RenderPlane(planeMatrix, terrain->mesh, terrain->texture, a_shader, cam, 20.0f);
	Matrix44 coliseoMatrix = Matrix44();
	coliseoMatrix.translate(0.0f, -2.0f, 0.0f);
	coliseoMatrix.scale(75.0f, 75.0f, 75.0f);
	RenderMesh(coliseoMatrix, colosseum->mesh, colosseum->texture, a_shader, cam);
	//GUI RENDER

	// RenderAllGUI
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	//RenderGUI(50,50,100,100,Texture::Get("data/grass.tga"));

	if (RenderButton(positions[0].x, positions[0].y, 250, 75, a_shader, textures[0], Vector4(0, 0, 1, 1), textures_hover[0]))
	{
		Game::instance->scene = GAME;
		DATA load_game = loadGame("data/saveFiles/file1.stats");
		if (load_game.modified) {
			Game::instance->game_s->stats = load_game.player_stats;
			Game::instance->game_s->player->pos = load_game.playerPosition;
			Game::instance->game_s->player->yaw = load_game.playerYaw;
		}
		else{
			Game::instance->game_s->player->pos = Vector3(30.12f, 0.0f, 16.88f);
			Game::instance->game_s->player->yaw = -249.8f;
		}
		//cout << "Let's game" << endl;
	}
	else if (RenderButton(positions[1].x, positions[1].y, 250, 75, a_shader, textures[1], Vector4(0, 0, 1, 1), textures_hover[1]))
	{
		Game::instance->scene = CONTROLS;
		Game::instance->controls->cam = cam;
		//cout << "Scene change" << endl;
	}
	else if (RenderButton(positions[2].x, positions[2].y, 250, 75, a_shader, textures[2], Vector4(0, 0, 1, 1), textures_hover[2]))
	{
		//	cout << "Bye bye!" << endl;
		Game::instance->must_exit = true;
	}
	if (existsSavedFile("data/saveFiles/file1.stats"))
	{
		if (RenderButton(572, 300, 75, 75, a_shader, Texture::Get("data/iconTextures/Delete Button.png"), Vector4(0, 0, 1, 1),Texture::Get("data/iconTextures/Delete hover.png"))) {
			deleteSavedFile("data/saveFiles/file1.stats");
		}
	}
	RenderGUI(positions[3].x, positions[3].y, 805, 100, a_shader, Texture::Get("data/iconTextures/panel_Example1.png"), Vector4(0, 0, 1, 1));

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);


	//Render background

	//para pintar quad
	//pasar info al shader
	//hacer draw call
		
	//draw title
	drawText(75, 50, "Legends of the Colosseum", Vector3(0, 0, 0), 5);
	

	Camera::current = cam;
	
}

void IntroStage::update(float elapsed_time)
{
	//OPTION SELECTION
	float speed = elapsed_time * 70.0f; //the speed is defined by the seconds_elapsed so it goes constant

	//example

	//mouse input to rotate the cam
	cam->rotate(elapsed_time * rotationSpeedIntro, Vector3(0.0f, -1.0f, 0.0f));


	//if (Input::wasKeyPressed(SDL_SCANCODE_9)) cout << "Camera rotation speed = " << rotationSpeedIntro << endl;
	//async input to move the camera around
    cam->move(Vector3(-1.0f, 0.0f, 0.0f) * speed);
	posx = Input::mouse_position.x;
	posy = Input::mouse_position.y;
	if (Input::wasKeyPressed(SDL_SCANCODE_9)) cout << posx << "," << posy << endl;

	if (Input::wasKeyPressed(SDL_SCANCODE_F5)) reloadIcons();
}

//ControlsStage
#pragma region COMPLEMENTARY_ICON_FUNCTIONS
void ControlsStage::ReloadIcons() {
	icons.clear();
	positions.clear();
	icons = get_all_files_names_within_folder();
	for (size_t i = 0; i < icons.size(); i++)
	{
		positions.push_back(readPosition(icons[i].c_str()));
	}
}
#pragma endregion COMPLEMENTARY_ICON_FUNCTIONS
void ControlsStage::render()
{

	glDisable(GL_DEPTH_TEST);
	sky->render();
	glEnable(GL_DEPTH_TEST);
	Matrix44 planeMatrix = Matrix44();
	RenderPlane(planeMatrix, terrain->mesh, terrain->texture, a_shader, cam, 20.0f);
	Matrix44 coliseoMatrix = Matrix44();
	coliseoMatrix.translate(0.0f, -2.0f, 0.0f);
	coliseoMatrix.scale(75.0f, 75.0f, 75.0f);
	RenderMesh(coliseoMatrix, colosseum->mesh, colosseum->texture, a_shader, cam);
	
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	//RenderGUI(50,50,100,100,Texture::Get("data/grass.tga"));
	RenderGUI(403, 275, 805, 800, a_shader, Texture::Get("data/iconTextures/panel_Example1.png"), Vector4(0, 0, 1, 1));
	for (size_t i = 0; i < icons.size(); i++)
	{
		RenderGUI(positions[i].x, positions[i].y, 50, 50, a_shader, textures[0], Vector4(0, 0, 1, 1));
	}
	float move_mouse_x = 185.85, move_mouse_y = 406.851;
	
	RenderGUI(548.3, 259.201, 25, 25, a_shader, Texture::Get("data/controlsIconsTextures/Left_arrow.png"), Vector4(0, 0, 1, 1));
	RenderGUI(651.697, 259.201, 25, 25, a_shader, Texture::Get("data/controlsIconsTextures/Right_arrow.png"), Vector4(0, 0, 1, 1));
	RenderGUI(599.898, 259.201, 25, 25, a_shader, Texture::Get("data/controlsIconsTextures/Up_arrow.png"), Vector4(0, 0, 1, 1));
	RenderGUI((move_mouse_x + sin(2.0f * Game::instance->time) * 25.0f), 406.851, 100, 100, a_shader, Texture::Get("data/controlsIconsTextures/mouse_icon.png"), Vector4(0, 0, 1, 1));
	bool click = 1.5f * sin(2.0f * Game::instance->time) > 0;
	if(!click) RenderGUI(586.699, 406.851, 100, 100, a_shader, Texture::Get("data/controlsIconsTextures/mouse_icon.png"), Vector4(0, 0, 1, 1));
	else RenderGUI(586.699, 406.851, 100, 100, a_shader, Texture::Get("data/controlsIconsTextures/mouse__left_click_icon.png"), Vector4(0, 0, 1, 1));
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	drawText(300, 70, "Controls", Vector3(0, 0, 0), 5);
	drawText(112.45f, 130.8f, "Movement:", Vector3(0, 0, 0), 3);
	drawText(542.6, 130.75, "Actions:", Vector3(0, 0, 0), 3);
	drawText(127.55, 324.151, "Camera:", Vector3(0, 0, 0), 3);
	drawText(543.4, 324.151, "Attack:", Vector3(0, 0, 0), 3);
	drawText(175.8, 194.3, "W", Vector3(0, 0, 0), 3);
	drawText(177.45, 249.451, "S", Vector3(0, 0, 0), 3);
	drawText(127.5, 249.451, "A", Vector3(0, 0, 0), 3);
	drawText(230, 249.451, "D", Vector3(0, 0, 0), 3);
	drawText(581.749, 197.05, "Esc", Vector3(0, 0, 0), 2.5);

}

void ControlsStage::update(float elapsed_time)
{
	//mouse input to rotate the cam
	float speed = elapsed_time * 70.0f; //the speed is defined by the seconds_elapsed so it goes constant

	//example

	//mouse input to rotate the cam
	cam->rotate(elapsed_time * rotationSpeedIntro, Vector3(0.0f, -1.0f, 0.0f));

	if (Input::isKeyPressed(SDL_SCANCODE_UP)) posy -= 50.0f * elapsed_time;
	if (Input::isKeyPressed(SDL_SCANCODE_DOWN)) posy += 50.0f * elapsed_time;
	if(Input::isKeyPressed(SDL_SCANCODE_LEFT)) posx -= 50.0f * elapsed_time;
	if(Input::isKeyPressed(SDL_SCANCODE_RIGHT)) posx += 50.0f * elapsed_time;

	if (Input::wasKeyPressed(SDL_SCANCODE_9)) cout << "Position: " << posx << ", " << posy << endl;
	//if (Input::wasKeyPressed(SDL_SCANCODE_RETURN)) cin >> text;
	//async input to move the camera around
	cam->move(Vector3(-1.0f, 0.0f, 0.0f) * speed);

	if (Input::wasKeyPressed(SDL_SCANCODE_F5)) ReloadIcons();
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
#pragma region UI
	//Render UI player
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (!menu) {
		RenderGUI(209.25 - ((stats.missing_hp * 290) / 2), 45.9, 290 - (stats.missing_hp * 290), 20, gui_shader, textures[1], Vector4(0, 0, 1, 1));
		RenderGUI(209.25, 45.9, 300, 25, gui_shader, textures[0], Vector4(0, 0, 1, 1));
	}
	else {
		RenderGUI(403, 300, 805, 800, gui_shader, Texture::Get("data/iconTextures/panel_Example1.png"), Vector4(0, 0, 1, 1));
		if (RenderButton(400, 220, 400, 100, gui_shader, Texture::Get("data/iconTextures/panel_Example1.png"), Vector4(0, 0, 1, 1), Texture::Get("data/iconTextures/panel_Example2.png"))) {
			menu = !menu;
		} //RESUME BUTTON
		if (RenderButton(400, 307, 400, 100, gui_shader, Texture::Get("data/iconTextures/panel_Example1.png"), Vector4(0, 0, 1, 1), Texture::Get("data/iconTextures/panel_Example2.png"))) {
			//Call save function
			DATA current_data = {
				{
				stats.strength,
				stats.missing_hp,
				stats.resistance
				},
				player->pos,
				player->yaw
			};
			saveGame("file1",current_data);
			cout << "Game Saved!" << endl;
		} //SAVE BUTTON
		if (RenderButton(400, 393, 400, 100, gui_shader, Texture::Get("data/iconTextures/panel_Example1.png"), Vector4(0, 0, 1, 1), Texture::Get("data/iconTextures/panel_Example2.png"))) {
			Game::instance->scene = INTRO;
			Game::instance->intro->cam->lookAt(Vector3(148.92f, 77.76f, 57.58f), Vector3(30.0f, 21.99f, 9.88f), Vector3(0, 1, 0));
			menu = false;
		}//MAIN MENU BUTTON
		
		
		drawText(134, 94, "GAME PAUSED", Vector3(0, 0, 0), 8);
		drawText(316, 201, "Resume", Vector3(0, 0, 0), 5);
		drawText(271, 284, "Save game", Vector3(0, 0, 0), 5);
		drawText(276, 375, "Main Menu", Vector3(0, 0, 0), 5);
		//drawText(posx, posy, "Save", Vector3(0, 0, 0), 5);
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
#pragma endregion UI

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
			else {
				player->pos = Vector3(30.12f, 0.0f, 16.88f);
				player->yaw = -249.8f;
			}
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
	if (!menu) {
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
							currentEnemy->hp -= stats.strength;
							cout << "Enemy current health: " << currentEnemy->hp << endl;
							weapon.attacked = false;
						}
					}
					else {
						POSITION nextAttack = currentEnemy->GetNextAttack();
						//Animaciones enemigo
						if (nextAttack == UP) {
							//Animacion up 
							parried = true;
							cout << "UP" << endl;
						}
						else if (nextAttack == LEFT) {
							//Animacion left
							parried = true;
							cout << "LEFT" << endl;
						}
						else {
							//Animacion right
							parried = false;
							cout << "RIGHT" << endl;
						}
						if (!parried) {
							stats.missing_hp += 1 - stats.resistance;
						}
						parried = false;
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
						currentEnemy = new EnemyAI(5, enemies[i], 2);
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
		if (stats.missing_hp >= 1.0f) {
			stats.missing_hp = 1.0f;
			//Game::instance->scene = GAMEOVER;
		}
	}
#pragma endregion PLAYER_MOVEMENT

#pragma region CAMERA_MOVEMENT
	SDL_ShowCursor(menu);
	if (!menu) {
		player->yaw += -Input::mouse_delta.x * 5.0f * elapsed_time;
		Input::centerMouse();
	}
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
#pragma endregion BARTENDER_BEHAVIOUR

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