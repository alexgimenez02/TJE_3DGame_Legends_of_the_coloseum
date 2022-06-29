#include "stage.h"
#include "game.h"
#include "input.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include "animation.h"


GLenum error;
float rotationSpeedIntro  = 0.55f;
float posx, posy, posz;
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
			if (!newEntity->mesh) {
				newEntity->mesh = new Mesh();
				newEntity->mesh->createCube();
			}
			newEntity->texture = Texture::Get(texPath.c_str());
			if (!newEntity->texture) newEntity->texture = Texture::getBlackTexture();
			newEntity->name = meshName;
			newEntity->pos = MeshPosition;
			if (fileName == "data/ArenaJordiAlex.scene" && meshName != "WALL")
			{
				cout << "Arena" << endl;
				newEntity->model.scale(2.5f, 2.5f, 2.5f);
				newEntity->model.translate(0.0f, -0.02f, 0.0f);
				
			}
			bool roof = meshName == "roof_raised.obj" || meshName == "roof_flat.obj" || meshName == "door_frame.obj";
			if (!roof && fileName != "data/ArenaJordiAlex.scene") {
				entitiesWithColision->push_back(newEntity);
			}
			if (meshName == "WALL")
				entitiesWithColision->push_back(newEntity);
			else
				entities->push_back(newEntity);
		}
	}
}
void LoadEnemiesInScene(const char* fileName, vector<EntityMesh*> *enemies) //Loads enemies in file to scene
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
			int id = sceneParser.getint();

			string meshPath = "data/playermodels/" + meshName;
			string texPath = "data/playermodels/" + texName;
			newEntity->mesh = Mesh::Get(meshPath.c_str());
			if (!newEntity->mesh) newEntity->mesh = new Mesh();
			newEntity->texture = Texture::Get(texPath.c_str());
			if (!newEntity->texture) newEntity->texture = Texture::getBlackTexture();
			newEntity->name = meshName;
			newEntity->pos = MeshPosition;
			newEntity->scale = meshScale;
			newEntity->id = id;
			enemies->push_back(newEntity);
		}
	}
}
//Render specified mesh
void RenderMesh(Matrix44 model, Mesh* a_mesh, Texture* tex, Shader* a_shader, Camera* cam) {
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
	a_mesh->render(GL_TRIANGLES);

	//disable shader
	a_shader->disable();
#ifdef _DEBUG
	if (Camera::current) a_mesh->renderBounding(model);
#endif

}
//Renders the plane
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
		a_shader->setUniform("u_model", m);
		//do the draw call
		a_mesh->render(GL_TRIANGLES);


		//disable shader
		a_shader->disable();
	}
}
//Renders the GUI element
void RenderGUI(float x, float y, float w, float h, Shader* a_shader, Texture* tex, Vector4 tex_range, Vector4 color = Vector4(1, 1, 1, 1), bool flipUV = false) {
	
	int windowWidth = Game::instance->window_width; //Get the windowWidth
	int windowHeight = Game::instance->window_height; //And height
	
	Mesh quad;
	quad.createQuad(x, y, w, h, flipUV); //Create the quad

	Camera cam2D;
	cam2D.setOrthographic(0, windowWidth, windowHeight, 0, -1, 1); //The 2D camera

	if (!a_shader)return;
	a_shader->enable();

	a_shader->setUniform("u_color", color);
	a_shader->setUniform("u_viewprojection", cam2D.viewprojection_matrix);

	if(tex != NULL) a_shader->setUniform("u_texture", tex, 0);

	a_shader->setUniform("u_time", time);
	a_shader->setUniform("u_tex_range", tex_range);	
	a_shader->setUniform("u_model", Matrix44());
	quad.render(GL_TRIANGLES); //Renders the quad

	a_shader->disable();
}
//Renders the button in GUI
bool RenderButton(float x, float y, float w, float h, Shader* a_shader, Texture* tex,Vector4 tex_range, Texture* text_hover = NULL, bool flipUV = false)
{
	Vector2 mouse = Input::mouse_position; //Check the mouse position
	float halfWidth = w * 0.5f;
	float halfHeight = h * 0.5f;
	float min_x = x - halfWidth;
	float max_x = x + halfWidth;
	float min_y = y - halfHeight;
	float max_y = y + halfHeight;


	bool hover = mouse.x >= min_x && mouse.x <= max_x && mouse.y >= min_y && mouse.y <= max_y; //Check if the mouse is on top of the button
	if (text_hover != NULL) {
		Texture* texture = hover ? text_hover : tex;
		RenderGUI(x, y, w, h, a_shader, texture, tex_range, Vector4(1,1,1,1), flipUV); //If it has a texture for hover, use it
	}
	else {
		Vector4 color = hover ? Vector4(1, 1, 1, 1) : Vector4(1, 1, 1, 0.7f); //If it does not have extra texture, just change the color 
		RenderGUI(x, y, w, h, a_shader, tex, tex_range, color, flipUV);
	}

	bool pressed = Game::instance->wasLeftButtonPressed && hover; //Check if it was pressed
	return pressed;
}
//Renders the animations
void animate(Shader* a_shader, Animation* attack, Mesh* mesh, EnemyAI* currentEnemy, float durationTime, Camera* cam) {

	Matrix44 enemyModel = Matrix44();
	enemyModel.translate(currentEnemy->enemyEntity->pos.x, currentEnemy->enemyEntity->pos.y, currentEnemy->enemyEntity->pos.z);
	enemyModel.scale(currentEnemy->enemyEntity->scale, currentEnemy->enemyEntity->scale, currentEnemy->enemyEntity->scale);
	enemyModel.rotate(currentEnemy->enemyEntity->yaw * DEG2RAD, Vector3(0, 1, 0));

	a_shader->enable();
	attack->assignTime(durationTime + 0.6f);
	a_shader->setUniform("u_color", Vector4(1, 1, 1, 1));
	a_shader->setUniform("u_viewprojection", cam->viewprojection_matrix);
	if (currentEnemy->enemyEntity->texture != NULL) {
		a_shader->setUniform("u_texture", currentEnemy->enemyEntity->texture, 0);
	}
	a_shader->setUniform("u_tex_tiling", 1.0f);
	a_shader->setUniform("u_model", enemyModel);
	mesh->renderAnimated(GL_TRIANGLES, &attack->skeleton);
	a_shader->disable();
	currentEnemy->enemyEntity->model = enemyModel;
}
void animateWeapon(Shader* a_shader, Animation* attack, Matrix44 weaponModel, Matrix44 enemyModel, Camera* cam, sWeapon weapon) {
	Mesh* mesh = weapon.entity->mesh;
	weaponModel.scale(30, 30 , 30);
	//swordModel.rotate(180.0f * DEG2RAD, Vector3(0, 1, 0));
	Matrix44 idleHandLocalMatrix = attack->skeleton.getBoneMatrix("mixamorig_RightHand", false);
	Matrix44 Total = idleHandLocalMatrix * enemyModel;
	Total.scale(1 / 1.75f, 1 / 1.75f, 1 / 1.75f);
	RenderMesh(Total, weapon.entity->mesh, weapon.entity->texture, a_shader, cam);
}
#pragma region COLISION
COL_RETURN CheckColision(Vector3 pos, Vector3 nexPos, EntityMesh* entity,float elapsed_time, float radius = 2.0f) //Checks the colisions
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
	Vector3 push_away = normalize(coll - character_center) * elapsed_time;
	nexPos = (pos - push_away); //move to previous pos but a little bit further
	//cuidado con la Y, si nuestro juego es 2D la ponemos a 0
	nexPos.y = 0;
	
	ret.colision = true;
	ret.modifiedPosition = nexPos;
	return ret;
}
//Checks if the ray has colided with the Barman mesh
bool CheckRayWithBarman(Camera* cam, EntityMesh* barman)
{
	Vector2 mouse = Input::mouse_position;
	Game* g = Game::instance;
	Vector3 dir = cam->getRayDirection(mouse.x, mouse.y, g->window_width, g->window_height);
	Vector3 rayOrigin = cam->eye;
	Vector3 pos;
	Vector3 normal;
	return barman->mesh->testRayCollision(barman->model, rayOrigin, dir, pos, normal, 3.5f);
}
#pragma endregion COLISION

#pragma region CROSSHAIR
void drawCrosshair() //Draws the crosshair in the center of the screen
{
	float scale = 5;
	int half_width = Game::instance->window_width / 2;
	int half_heigth = Game::instance->window_height / 2;
	drawText(half_width - 14, half_heigth - 5, "+", Vector3(0, 0, 0), scale);
}
#pragma endregion CROSSHAIR	

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

//IntroStage
void IntroStage::render()
{

	glDisable(GL_DEPTH_TEST);
	sky->render(); //Render the skybox
	glEnable(GL_DEPTH_TEST);
	Matrix44 planeMatrix = Matrix44(); //Plane matrix
	RenderPlane(planeMatrix, terrain->mesh, terrain->texture, terrain->shader, cam, 200.0f); //Renders the plane
	Matrix44 coliseoMatrix = Matrix44(); //Coliseum matrix
	coliseoMatrix.translate(0.0f, -2.0f, 0.0f);
	coliseoMatrix.scale(75.0f, 75.0f, 75.0f); //Set it in the position and scale
	RenderMesh(coliseoMatrix, colosseum->mesh, colosseum->texture, terrain->shader, cam); //Renders the coliseum
	//GUI RENDER

	// RenderAllGUI
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (RenderButton(positions[0].x, positions[0].y, 250, 75, a_shader, textures[0], Vector4(0, 0, 1, 1), textures_hover[0])) //Render the button for "Play"
	{
		Game::instance->scene = GAME;
		DATA load_game = loadGame("data/saveFiles/file1.stats");
		if (load_game.modified) {
			Game::instance->game_s->stats = load_game.player_stats;
			Game::instance->game_s->player->pos = load_game.playerPosition;
			Game::instance->game_s->player->yaw = load_game.playerYaw;
			Game::instance->game_s->Stage_ID = load_game.curr_stage;
			Game::instance->game_s->diff = load_game.diff;
			Game::instance->game_s->lvl = load_game.battleFile;
		}
		else{
			Game::instance->InitGameStage();
		}
	}

	else if (RenderButton(positions[1].x, positions[1].y, 250, 75, a_shader, textures[1], Vector4(0, 0, 1, 1), textures_hover[1])) //Renders the "Controls" button
	{
		Game::instance->scene = CONTROLS;
		Game::instance->controls->cam = cam;
	}
	else if (RenderButton(positions[2].x, positions[2].y, 250, 75, a_shader, textures[2], Vector4(0, 0, 1, 1), textures_hover[2])) //Renders the "Exti" button
	{
		Game::instance->must_exit = true;
	}
	if (existsSavedFile("data/saveFiles/file1.stats")) //If exists a save file
	{
		//Render the button to delete it
		if (RenderButton(572, 300, 75, 75, a_shader, Texture::Get("data/iconTextures/Delete Button.png"), Vector4(0, 0, 1, 1),Texture::Get("data/iconTextures/Delete hover.png"))) {
			deleteSavedFile("data/saveFiles/file1.stats");
		}
	}
	RenderGUI(positions[3].x, positions[3].y, 805, 100, a_shader, Texture::Get("data/iconTextures/panel_Example1.png"), Vector4(0, 0, 1, 1)); //Renders the panel that contains the title

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
		
	//draw title
	drawText(75, 50, "Legends of the Colosseum", Vector3(0, 0, 0), 5); //Renders the title text
	
	Camera::current = cam; //Changes the camera to the current for each iteration
	
}

void IntroStage::update(float elapsed_time)
{
	float speed = elapsed_time * 70.0f; //the speed is defined by the seconds_elapsed so it goes constant
	//Camera orbitation
	cam->rotate(elapsed_time * rotationSpeedIntro, Vector3(0.0f, -1.0f, 0.0f)); //Rotate the camera in the y axis
    cam->move(Vector3(-1.0f, 0.0f, 0.0f) * speed); //Move the camera so it does not rotate in the same position and so it orbits
#ifdef _DEBUG
	if (Input::wasKeyPressed(SDL_SCANCODE_F5)) reloadIcons(); //Only for debug
#endif
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
void ControlsStage::render() //Follows the same principle as the Intro stage, just a background scene with UI
{

	glDisable(GL_DEPTH_TEST);
	sky->render(); //Render the sky
	glEnable(GL_DEPTH_TEST);
	Matrix44 planeMatrix = Matrix44();
	RenderPlane(planeMatrix, terrain->mesh, terrain->texture, a_shader, cam, 20.0f); //Render the plane
	Matrix44 coliseoMatrix = Matrix44();
	coliseoMatrix.translate(0.0f, -2.0f, 0.0f);
	coliseoMatrix.scale(75.0f, 75.0f, 75.0f);
	RenderMesh(coliseoMatrix, colosseum->mesh, colosseum->texture, a_shader, cam); //And the coliseum
	
	//Everything in the same position as the Intro stage left it
	//Render All GUI
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	RenderGUI(403, 275, 805, 800, a_shader, Texture::Get("data/iconTextures/panel_Example1.png"), Vector4(0, 0, 1, 1)); //Renders the panel that contains everything
	for (size_t i = 0; i < icons.size(); i++)
	{
		RenderGUI(positions[i].x, positions[i].y, 50, 50, a_shader, textures[0], Vector4(0, 0, 1, 1)); //Add each little icon to the position
	}
	float move_mouse_x = 185.85; //Adds the position for the animated mouse icon
	
	RenderGUI(548.3, 259.201, 25, 25, a_shader, Texture::Get("data/controlsIconsTextures/Left_arrow.png"), Vector4(0, 0, 1, 1)); //Adds the arrows for the controls
	RenderGUI(651.697, 259.201, 25, 25, a_shader, Texture::Get("data/controlsIconsTextures/Right_arrow.png"), Vector4(0, 0, 1, 1));
	RenderGUI(599.898, 259.201, 25, 25, a_shader, Texture::Get("data/controlsIconsTextures/Up_arrow.png"), Vector4(0, 0, 1, 1));
	//Moves the mouse icon
	RenderGUI((move_mouse_x + sin(2.0f * Game::instance->time) * 25.0f), 406.851, 100, 100, a_shader, Texture::Get("data/controlsIconsTextures/mouse_icon.png"), Vector4(0, 0, 1, 1));
	bool click = 1.5f * sin(2.0f * Game::instance->time) > 0; //Swaps between the clicks and unclicked mouse
	if(!click) RenderGUI(586.699, 406.851, 100, 100, a_shader, Texture::Get("data/controlsIconsTextures/mouse_icon.png"), Vector4(0, 0, 1, 1));
	else RenderGUI(586.699, 406.851, 100, 100, a_shader, Texture::Get("data/controlsIconsTextures/mouse__left_click_icon.png"), Vector4(0, 0, 1, 1));
	//Volume manager (only music)
	bool mute = Game::instance->volume == 0.0f;
	//Mute button
	if (mute) {
		if (RenderButton(387, 295, 80, 80, a_shader, Texture::Get("data/controlsIconsTextures/Audio col_Square Button.png"), Vector4(0, 0, 1, 1))) {
			Game::instance->volume = previous_volume; 
		}
	}
	else {
		if (RenderButton(387, 295, 80, 80, a_shader, Texture::Get("data/controlsIconsTextures/Audio Square Button.png"), Vector4(0, 0, 1, 1))){
			previous_volume = Game::instance->volume;
			Game::instance->volume = 0.0f;
		}
	}
	//Lower volume button
	if (mute) {
		RenderGUI(387, 378, 80, 80, a_shader, Texture::Get("data/controlsIconsTextures/Down col_Square Button.png"), Vector4(0, 0, 1, 1));
	}
	else {
		if (RenderButton(387, 378, 80, 80, a_shader, Texture::Get("data/controlsIconsTextures/Down Square Button.png"), Vector4(0, 0, 1, 1))) {
			Game::instance->volume -= 0.1f;
			if (Game::instance->volume <= 0.0f) Game::instance->volume = 0.0f;
		}
	}
	//Higher volume button
	if (Game::instance->volume == 1.0f) {
		RenderGUI(387, 208, 80, 80, a_shader, Texture::Get("data/controlsIconsTextures/Up col_Square Button.png"),Vector4(0,0,1,1));
	}
	else {
		if (RenderButton(387, 208, 80, 80, a_shader, Texture::Get("data/controlsIconsTextures/Up Square Button.png"), Vector4(0, 0, 1, 1))) {
			Game::instance->volume += 0.1f;
			if (Game::instance->volume >= 1.0f) Game::instance->volume = 1.0f;
		}

	}
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	//Draws all the text in screen
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
	float speed = elapsed_time * 70.0f; //the speed is defined by the seconds_elapsed so it goes constant

	cam->rotate(elapsed_time * rotationSpeedIntro, Vector3(0.0f, -1.0f, 0.0f)); //Same as intro stage
	cam->move(Vector3(-1.0f, 0.0f, 0.0f) * speed);
	
#ifdef _DEBUG
	posx = Input::mouse_position.x;
	posy = Input::mouse_position.y;
	if (Input::wasKeyPressed(SDL_SCANCODE_9)) cout << posx << "," << posy << endl;
	if (Input::wasKeyPressed(SDL_SCANCODE_F5)) ReloadIcons();
#endif
}





//GameStage
void GameStage::render()
{
	//set flags
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	Camera* cam = Game::instance->camera; //Gets the camera
	glDisable(GL_DEPTH_TEST);
	sky->render(); //Renders the sky
	glEnable(GL_DEPTH_TEST);

	if (!entities.empty()) //Check if the entities list is empty
	{	//Renders the entities in the scene
		for (size_t i = 0; i < entities.size(); i++)
			RenderMesh(entities[i]->model, entities[i]->mesh, entities[i]->texture, shader, cam);
	}

	Texture* tex = Texture::Get("data/playermodels/Character.png"); //Gets the texture

	Shader* a_shader = Shader::Get("data/shaders/skinning.vs", "data/shaders/texture.fs"); //And the shader

	Matrix44 playerModel; //Get the model for the player (where the camera is)
	playerModel.translate(player->pos.x, player->pos.y, player->pos.z);
	playerModel.rotate(player->yaw * DEG2RAD, Vector3(0, 1, 0)); //Move and rotate depending on where the player is looking

	Matrix44 camModel = playerModel; //get the camera mode
	camModel.rotate(player->pitch * DEG2RAD, Vector3(1, 0, 0)); //Set the rotation
	Vector3 eye = playerModel * Vector3(0.0f, 0.45f, 0.0f); //The eye
	if(Stage_ID == TABERN)
		eye = playerModel * Vector3(0.0f, 2.45f, 0.0f);
	Vector3 center = cam->center; //Center
	if (!yAxisCam)
		center = eye + camModel.rotateVector(Vector3(0, 0, 1));
	Vector3 up = camModel.rotateVector(Vector3(0, 1, 0)); //And up
	cam->enable(); //Enable it
	cam->lookAt(eye, center, up); //Change where it looks at
	Matrix44 swordModel; //Create the swordmodel
	Vector3 swordOffset = weapon.weaponOffset; //Add the offset
	swordModel.setTranslation(swordOffset.x, swordOffset.y, swordOffset.z);
	swordModel.rotate(-90.0f * DEG2RAD, Vector3(0, 1, 0));
	swordModel.scale(1 / 25.0f, 1 / 25.0f, 1 / 25.0f);
	swordModel = swordModel * playerModel; //Make it move with the player model matrix
	//Player weapon animations
	if (weapon.defType!= NONE) //Defence animations
	{
		if (weapon.defType== UP) //Up animation
		{
			//motion defence up
			swordModel.translate(0.0f, weapon.defenceMotionUp, weapon.defenceMotion);
			swordModel.rotate(weapon.defenceRotation * DEG2RAD, Vector3(1, 0, 0));
		}
		else //Left and right animations
		{
			swordModel.translate(0.0f, 0.0f, weapon.defenceMotion);
		}
	}
	if (weapon.movementMotion) //When moving
	{
		Matrix44 T;
		T.setTranslation(0.0f, 0.025f * sin(Game::instance->time) + 0.05f, 0.0f); //Oscilates up and down
		swordModel = swordModel * T;
	}
	swordModel.rotate(weapon.attackMotion * DEG2RAD, Vector3(0, 0, 1)); //And rotates in chase needed when attacking up
	if(Stage_ID != TABERN) RenderMesh(swordModel, weapon.entity->mesh, weapon.entity->texture, shader, cam); //If the stage is the tabern, there is no sword
	if (Stage_ID == MAP) {
		if (list.first || list.third || obj == LVLUP) { //Renders the objective arrows
			if (sphereTabern) {
				Matrix44 T;
				T.setTranslation(0.0f, 0.005f * sin(Game::instance->time), 0.0f); //Oscilate up and down
				sphereTabern->model = sphereTabern->model * T;
				RenderMesh(sphereTabern->model, sphereTabern->mesh, sphereTabern->texture, shader, cam); 
			}
		}
		else{ //Same for the arena
			if (sphereArena) {
				Matrix44 T;
				T.setTranslation(0.0f, 0.005f * sin(Game::instance->time), 0.0f);
				sphereArena->model = sphereArena->model * T;
				RenderMesh(sphereArena->model, sphereArena->mesh, sphereArena->texture, shader, cam);
			}
		}
	}
	if (Stage_ID == ARENA) //If the stage is the arena
	{ //Get the animation
		sANIMATION anim = Game::instance->sanimation;
		for (size_t i = 0; i < enemies.size(); i++) //Render the enemies into the idle animation
		{
			int x = enemies[i]->id;
			Matrix44 enemyModel = Matrix44();
			enemyModel.translate(enemies[i]->pos.x, enemies[i]->pos.y, enemies[i]->pos.z);
			enemyModel.scale(enemies[i]->scale, enemies[i]->scale, enemies[i]->scale);
			enemyModel.rotate(enemies[i]->yaw * DEG2RAD, Vector3(0, 1, 0));
			enemies[i]->model = enemyModel;
			a_shader->enable();
			Texture* tex = enemies[i]->texture;
			
			anim.idle_attack[x]->assignTime(Game::instance->time);
			a_shader->setUniform("u_color", Vector4(1, 1, 1, 1));
			a_shader->setUniform("u_viewprojection", cam->viewprojection_matrix);
			if (tex != NULL) {
				a_shader->setUniform("u_texture", tex, 0);
			}
			a_shader->setUniform("u_tex_tiling", 1.0f);
			a_shader->setUniform("u_model", enemies[i]->model);

			if (currentEnemy != NULL) {

				if (enemies[i] != currentEnemy->enemyEntity) {
					
					anim.idle_mesh[x]->renderAnimated(GL_TRIANGLES, &anim.idle_attack[x]->skeleton);
					animateWeapon(a_shader, anim.idle_attack[x], swordModel, enemyModel, cam, weapon);
				}
			}
			else {
				anim.idle_mesh[x]->renderAnimated(GL_TRIANGLES, &anim.idle_attack[x]->skeleton);
				animateWeapon(a_shader, anim.idle_attack[x], swordModel, enemyModel, cam, weapon);
			}

			a_shader->disable();

		}
		if (currentEnemy != NULL) { //If there is a fight
			 //Get the animation for each attack
			if (Attack == DOWN) { //Damage animation
				int x = currentEnemy->enemyEntity->id;
				animate(a_shader, anim.down_attack[x], anim.down_mesh[x], currentEnemy, durationTime, cam);
				animateWeapon(a_shader, anim.down_attack[x], swordModel, currentEnemy->enemyEntity->model, cam, weapon);

			}
			else if (Attack == NONE && isBattle) { //Idle animation
				int x = currentEnemy->enemyEntity->id;
				animate(a_shader, anim.idle_attack[x], anim.idle_mesh[x], currentEnemy, durationTime, cam);
				animateWeapon(a_shader, anim.idle_attack[x], swordModel, currentEnemy->enemyEntity->model, cam, weapon);

			}
			else if (Attack == UP) { //Upper attack animation

				int x = currentEnemy->enemyEntity->id;
				animate(a_shader, anim.up_attack[x], anim.up_mesh[x], currentEnemy, durationTime, cam);
				animateWeapon(a_shader, anim.up_attack[x], swordModel, currentEnemy->enemyEntity->model, cam, weapon);

			}
			else if (Attack == LEFT) { //Left attack animation

				int x = currentEnemy->enemyEntity->id;
				animate(a_shader, anim.left_attack[x], anim.left_mesh[x], currentEnemy, durationTime, cam);
				animateWeapon(a_shader, anim.left_attack[x], swordModel, currentEnemy->enemyEntity->model, cam, weapon);

			}
			else if (Attack == RIGHT) { //Right attack animation

				int x = currentEnemy->enemyEntity->id;
				animate(a_shader, anim.right_attack[x], anim.right_mesh[x], currentEnemy, durationTime, cam);
				animateWeapon(a_shader, anim.right_attack[x], swordModel, currentEnemy->enemyEntity->model, cam, weapon);

			}
		}
	}
	if (Stage_ID == TABERN) //If stage is Tabern
	{
		if (barTender) { //Render the mesh for the NPC bartender
			Matrix44 npc_model = Matrix44();
			npc_model.translate(barTender->pos.x, barTender->pos.y, barTender->pos.z); 
			npc_model.scale(barTender->scale, barTender->scale, barTender->scale);
			npc_model.rotate(barTender->yaw * DEG2RAD, Vector3(0, 1, 0));
			barTender->model = npc_model;
			RenderMesh(npc_model, barTender->mesh, barTender->texture, shader, cam); //No animation
		}
	}
	RenderPlane(terrain->model, terrain->mesh, terrain->texture, terrain->shader, cam, tiling); //Render the plane
	drawCrosshair(); //Draws the crosshair
#pragma region UI
	//Render UI player
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (!menu) { //Right corner panel with objective
		if (list.first || list.second || list.third || (obj == BATTLE && Stage_ID != ARENA) || (obj == LVLUP && Stage_ID != ARENA)) {
			if(!interaction) RenderGUI(588, 62, 400, 100, gui_shader, Texture::Get("data/iconTextures/panel_Example1.png"), Vector4(0, 0, 1, 1)); //If you are not interacting, the panel keeps showing
		}
		if (interaction) //If you are interacting
			RenderGUI(397, 514, 800, 200, gui_shader, Texture::Get("data/iconTextures/panel_Example1.png"), Vector4(0, 0, 1, 1)); //The text panel appears
		if (list.third && interaction && nextText > 0) { //The buttons for the level up option
			//Yes button
			if (RenderButton(290, 537, 150, 80, gui_shader, Texture::Get("data/iconTextures/panel_Example1.png"), Vector4(0, 0, 1, 1), Texture::Get("data/iconTextures/panel_Example2.png"))) {
				list.third = false;
				if(!(stats.resistance == 0.5f && stats.strength == 5)) lvlUpMenu = true;
				nextText = 0;
				obj = LVLUP;
				interaction = false;
			}
			//No button
			if (RenderButton(471, 537, 150, 80, gui_shader, Texture::Get("data/iconTextures/panel_Example1.png"), Vector4(0, 0, 1, 1), Texture::Get("data/iconTextures/panel_Example2.png")))
			{
				nextText = 0;
				interaction = false;
			}
		}
		if (lvlUpMenu) { //Level up menu gui
			RenderGUI(403, 300, 805, 800, gui_shader, Texture::Get("data/iconTextures/panel_Example1.png"), Vector4(0, 0, 1, 1)); //The big panel
			if (stats.strength < 5) {
				//The button icons and each interaction
				if(RenderButton(200, 230, 100, 100, gui_shader, Texture::Get("data/gameIcons/biceps.png"), Vector4(0, 0, 1, 1)))
				{ 
					stats.strength++;
					lvlUpMenu = false;
					obj = BATTLE;
				}
			}
			else {
				//If max stat, then it becomes a GUI element only, and not a button
				RenderGUI(200, 230, 100, 100, gui_shader, Texture::Get("data/gameIcons/biceps.png"), Vector4(0, 0, 1, 1), Vector4(1, 1, 1, 0.7));
			}
			//Same for resistance
			if (stats.resistance < 0.5f)
			{
				if (RenderButton(200, 370, 100, 100, gui_shader, Texture::Get("data/gameIcons/muscular-torso.png"), Vector4(0, 0, 1, 1)))
				{
					stats.resistance += 0.1f;
					lvlUpMenu = false;
					obj = BATTLE;
				}
			}
			else {
				RenderGUI(200, 370, 100, 100, gui_shader, Texture::Get("data/gameIcons/muscular-torso.png"), Vector4(0, 0, 1, 1), Vector4(1, 1, 1, 0.7));
			}
			if(stats.missing_hp != 0.0f) stats.missing_hp = 0.0f; //Heals the player's hp
		}
	}
	if (!menu) { //Renders the health bar
		RenderGUI(209.25 - ((stats.missing_hp * 290) / 2), 45.9, 290 - (stats.missing_hp * 290), 20, gui_shader, textures[1], Vector4(0, 0, 1, 1));
		RenderGUI(209.25, 45.9, 300, 25, gui_shader, textures[0], Vector4(0, 0, 1, 1));
	}
	else {
		//If 'Esc' key is pressed
		//Big panel
		RenderGUI(403, 300, 805, 800, gui_shader, Texture::Get("data/iconTextures/panel_Example1.png"), Vector4(0, 0, 1, 1));
		//RESUME BUTTON
		if (RenderButton(400, 220, 400, 100, gui_shader, Texture::Get("data/iconTextures/panel_Example1.png"), Vector4(0, 0, 1, 1), Texture::Get("data/iconTextures/panel_Example2.png"))) {
			menu = !menu;
		} //SAVE BUTTON
		if (RenderButton(400, 307, 400, 100, gui_shader, Texture::Get("data/iconTextures/panel_Example1.png"), Vector4(0, 0, 1, 1), Texture::Get("data/iconTextures/panel_Example2.png"))) {
			//Call save function
			//Creates the DATA structure
			DATA current_data = {
				{
				stats.strength,
				stats.missing_hp,
				stats.resistance
				},
				player->pos,
				player->yaw,
				Stage_ID,
				diff,
				lvl
			};
			saveGame("file1",current_data); //Saves the game into a STATS file
#ifdef _DEBUG
			cout << "Game Saved!" << endl;
#endif		
		} 
		//MAIN MENU BUTTON
		if (RenderButton(400, 393, 400, 100, gui_shader, Texture::Get("data/iconTextures/panel_Example1.png"), Vector4(0, 0, 1, 1), Texture::Get("data/iconTextures/panel_Example2.png"))) {
			Game::instance->scene = INTRO;
			Game::instance->intro->cam->lookAt(Vector3(148.92f, 77.76f, 57.58f), Vector3(30.0f, 21.99f, 9.88f), Vector3(0, 1, 0)); //Resets the camera position
			if (!existsSavedFile("data/saveFiles/file1.stats")) { //And if there is no save file
				Game::instance->InitGameStage(); //Restart the Game
				Game::instance->game_s->Stage_ID = MAP;
			}
			//Booleans for diferent controlers (such as map swapping and menu)
			mapSwap = true; 
			menu = false;
		}
		//Draws the text on the menu
		drawText(134, 94, "GAME PAUSED", Vector3(0, 0, 0), 8);
		drawText(316, 201, "Resume", Vector3(0, 0, 0), 5);
		drawText(271, 284, "Save game", Vector3(0, 0, 0), 5);
		drawText(276, 375, "Main Menu", Vector3(0, 0, 0), 5);
	}
	if (Game::instance->scene != INTRO) { //If the main menu is not called
		if (!menu) { //And there is no menu
			if (obj == TUTORIAL) { //And the objective is the first fight
				if (list.first) { //First checkpoint
					if (Stage_ID == TABERN) { //Enter tabern
						if (interaction) { //When talking to the barman
							switch (nextText) {
							case 0: //First text
								drawText(85, 482, "Welcome to the Arena's town, where powerful\nwarriors go to the arena and test their skills", Vector3(0, 0, 0), 2.5);
								break;
							case 1: //Second text
								drawText(74, 483, "   You should go yourself and try it out.\nMaybe you are the new king of the Arena.", Vector3(0, 0, 0), 3);
								break;
							default: //After third click
								nextText = 0;
								interaction = false;
								list.second = true;
								list.first = false;
								break;
							}
						}
						else { //Objective text
							drawText(438, 53, "Talk to the barman", Vector3(0, 0, 0), 3);
						}
					}
					else {
						drawText(452, 54, "Go to the tabern", Vector3(0, 0, 0), 3); //If outside the tabern, objective text
					}
				}
				if (list.second) { //Second objective
					if (interaction) { //If reinteract with the barman
						drawText(165, 495, "GO TO THE ARENA!", Vector3(0, 0, 0), 5); //Text
						if (nextText > 0) { //Next click
							interaction = false;
							nextText = 0;
						}
					}
					else drawText(465, 50, "Go to the arena", Vector3(0, 0, 0), 3); //Objective text
					if (Stage_ID == ARENA) list.second = false; //When entering arena, third objectives starts
				}
		
				if (list.third) { //When finishing the fights
					if (Stage_ID == TABERN) { //On tabern
						if (interaction) { //When talking
							switch (nextText) {
							case 0: //First text
								drawText(97, 504, "Welcome back! Did you enjoy your fights?", Vector3(0, 0, 0), 2.7);
								break;
							default: //Level up text
								drawText(72, 482, "Do you want to upgrade your stats with a drink?", Vector3(0, 0, 0), 2.5);
								drawText(271, 530, "Yes", Vector3(0, 0, 0), 2);
								drawText(459, 530, "No", Vector3(0, 0, 0), 2);
								break;
							}
						}else //Objective text
						drawText(438, 53, "Talk to the barman", Vector3(0, 0, 0), 3);
					}else //Objective text
					drawText(424, 52, "Return to the tabern", Vector3(0, 0, 0), 3);
				}
			}
			else if (obj == LVLUP) { //Objetive level up
				if (Stage_ID == TABERN) {
					if(!lvlUpMenu) drawText(438, 53, "Talk to the barman", Vector3(0, 0, 0), 3); //Draws the objective text s
					if (lvlUpMenu) { //Level up Menu
						drawText(150, 140, "Strength beer", Vector3(0, 0, 0), 3); //Text for each option
						drawText(150, 290, "Resistance beer", Vector3(0, 0, 0), 3);
						//Strength
						if (stats.strength < 5) { //Text for knowing the level
							drawText(300, 216,"Current strength:",Vector3(0,0,0),3);
							drawText(585, 216,to_string(stats.strength),Vector3(0,0,0),3);

						}
						else { //If max level, the text changes
							drawText(345, 216,"Max strength",Vector3(0,0,0),3.5);
						}
						//Same for resistance
						if (stats.resistance < 0.5f) {
							drawText(300, 360,"Current resistance:",Vector3(0,0,0),3);
							int curr_res = stats.resistance * 10;
							drawText(607, 360,to_string(curr_res), Vector3(0, 0, 0), 3);
						}
						else {
							drawText(327, 358,"Max resistance",Vector3(0,0,0),3.5);
						}
					
					}
					else 
						if(!interaction) drawText(438, 53, "Talk to the barman", Vector3(0, 0, 0), 3); //objective text
				
				
				}
				else {
					drawText(452, 54, "Go to the tabern", Vector3(0, 0, 0), 3); //Objective text
				}
			}
			else {
				if(Stage_ID != ARENA) drawText(465, 50, "Go to the arena", Vector3(0, 0, 0), 3); //Objective text
				if (interaction) interaction = false; //Avoid softlocks
			}
		}
	}
	//Disable flags
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
#pragma endregion UI
	Camera::current = cam;
}

void GameStage::update(float elapsed_time)
{
	//Update time related variables
	durationTime += elapsed_time * 0.75; 
	cooldown -= elapsed_time * 0.75;
	parry += elapsed_time * 0.75;
	/*posx = Input::mouse_position.x;
	posy = Input::mouse_position.y;*/
#ifdef _DEBUG
	if (Input::isKeyPressed(SDL_SCANCODE_9)) cout << cooldown << endl;
#endif
#pragma region SCENE_LOADER
	if (mapSwap) //Allows map swapping
	{
		//clears the entities buffers
		entities.clear();
		entitiesColision.clear();
		switch (Stage_ID) { //Checks which map is the next to load
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
			sphereArena->pos = Vector3(0.57f, 0.25f, -12.66f);
			sphereTabern->mesh = Mesh::Get("data/arrow.obj");
			sphereTabern->texture = Texture::Get("data/arrow.png");
			sphereTabern->model = Matrix44();
			sphereTabern->model.translate(-11.005f, 0.25f, 2.67f);
			sphereTabern->pos = Vector3(-11.005f, 0.25f, 2.67f);
			terrain->texture = Texture::Get("data/grass.tga");
			tiling = 30.0f;
			Sleep(250); //Sleep to allow the load to be done
			break;
		case ARENA:
			enemies.clear();
			LoadSceneFile("data/ArenaJordiAlex.scene", &entities, &entitiesColision);
			if (lvl%3 == 0) { //Depending on the current battle
				LoadEnemiesInScene("data/enemies1.scene", &enemies);
			}
			else if (lvl % 3 == 1) {
				LoadEnemiesInScene("data/enemies2.scene", &enemies);
			}
			else {
				LoadEnemiesInScene("data/enemies3.scene", &enemies);
			}
			lvl++; //Changes the next battle
			diff++; //And the difficulty
			player->pos = Vector3(37.41f, 0.0f, -27.66f); 
			player->yaw = -435.6f;
			terrain->texture = Texture::Get("data/sand.tga"); //Changes the terrain texture
			tiling = 200.0f;
			Sleep(250);
			break;
		case TABERN:
			LoadSceneFile("data/TabernJordiAlex.scene", &entities, &entitiesColision);
			if (!barTender) {
				barTender = new EntityMesh();
				barTender->mesh = Mesh::Get("data/playermodels/Character7.obj"); //Adds the barman
				barTender->texture = Texture::Get("data/playermodels/Character.png");
				barTender->pos = Vector3(0.03f, 0.1f, 1.6f);
				barTender->scale = 0.25f;
				barTender->model = Matrix44();
			}
			player->pos = Vector3(0.01f,0.0f,-8.30f);
			player->yaw = -360.25f;
			Sleep(250);
			break;

		}
		mapSwap = false; //Avoid multiple map loading
	}
#pragma endregion SCENE_LOADER
#pragma region PLAYER_MOVEMENT
	if (!menu) { //If you are playing
		float playerSpeed = 5.0f * elapsed_time; //Player speed
		if (Stage_ID == MAP)
		{
			Matrix44 playerRotation;
			playerRotation.rotate(player->yaw * DEG2RAD, Vector3(0, 1, 0));
			Vector3 playerVel;
			Vector3 forward = playerRotation.rotateVector(Vector3(0, 0, -1));
			Vector3 right = playerRotation.rotateVector(Vector3(1, 0, 0));
#ifdef _DEBUG
			if (Input::isKeyPressed(SDL_SCANCODE_LSHIFT)) playerSpeed *= 2.0f;
#endif
			//Movement
			if (Input::isKeyPressed(SDL_SCANCODE_W)) playerVel = playerVel - (forward * playerSpeed);
			if (Input::isKeyPressed(SDL_SCANCODE_S)) playerVel = playerVel + (forward * playerSpeed);
			if (Input::isKeyPressed(SDL_SCANCODE_A)) playerVel = playerVel + (right * playerSpeed);
			if (Input::isKeyPressed(SDL_SCANCODE_D)) playerVel = playerVel - (right * playerSpeed);
			weapon.movementMotion = playerVel.x != 0 || playerVel.y != 0 || playerVel.z != 0;

			Vector3 nexPos = player->pos + playerVel;
			COL_RETURN ret;
			//Check colision with objective arrows
			if (CheckColision(player->pos, nexPos, sphereArena, elapsed_time, 1.5f).colision && (list.second || obj == BATTLE)) {
				mapSwap = true;
				previous_stage = Stage_ID;
				Stage_ID = ARENA;
			}
			else if (CheckColision(player->pos, nexPos, sphereTabern, elapsed_time).colision && (list.first || list.third ||obj == LVLUP)) {
				mapSwap = true;
				previous_stage = Stage_ID;
				Stage_ID = TABERN;
			}
			else {
				for (size_t i = 0; i < entities.size(); i++) //Check colision with props
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
#ifdef _DEBUG
			if (Input::isKeyPressed(SDL_SCANCODE_LSHIFT)) playerSpeed *= 2.0f;
#endif
			if (Input::isKeyPressed(SDL_SCANCODE_W)) playerVel = playerVel - (forward * playerSpeed);
			if (Input::isKeyPressed(SDL_SCANCODE_S)) playerVel = playerVel + (forward * playerSpeed);
			if (Input::isKeyPressed(SDL_SCANCODE_A)) playerVel = playerVel + (right * playerSpeed);
			if (Input::isKeyPressed(SDL_SCANCODE_D)) playerVel = playerVel - (right * playerSpeed);

			Vector3 nexPos = player->pos + playerVel;
			COL_RETURN ret;
			for (size_t i = 0; i < entitiesColision.size(); i++) //Checks colision with props (ignoring the ground or the roof
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
		else { //Arena 
			if (isBattle) //If battle
			{
				weapon.movementMotion = false; //Deactivate the weapon motion
				if (currentEnemy->hp > 0) { //If the enemy is alive
					if (currentEnemy->EmptyAttacks() && cooldown <= 0) { //has no attacks
						//Wait until hit
						Attack = NONE; //Idle
						if (weapon.attacked) {
							Attack = DOWN; //Damage
							currentEnemy->GenerateAttacks();
							currentEnemy->hp -= stats.strength;
#ifdef _DEBUG
							cout << "Enemy current health: " << currentEnemy->hp << endl;
#endif
							cooldown = 2.0f;
							waitTime = 0.0f;
							durationTime = 0.75f;
						}
						else if (waitTime >= 5.0f) { //If the player does no attack after 5 seconds
							currentEnemy->GenerateAttacks();
							waitTime = 0.0f;
						}
						else {
							waitTime += elapsed_time; //Add more to the wait time
						}
						
					}
					else if (cooldown <= 0) { //After the animation of the first attack
						POSITION nextAttack = currentEnemy->GetNextAttack();
						//Animaciones enemigo
						Attack = nextAttack;
#ifdef _DEBUG
						switch (Attack) {
						case UP: 
							cout << "UP" << endl;
							break;
						case RIGHT:
							cout << "RIGHT" << endl;
							break;
						case LEFT:
							cout << "LEFT" << endl;
							break;
						}
#endif
						durationTime = -0.25f;
						cooldown = 2.0f;
					}
					else if (cooldown <= 1.0f && cooldown > 0.0f && !(Attack == NONE || Attack == DOWN)) { //Player defensive position
						parried = Attack == weapon.defType;

						if (curr_SFX_channel != 0) //Stops the previous SFX sound
							Audio::Stop(curr_SFX_channel);

						switch (Attack) { //Depending on the attack the sound is different
						case UP:
							if (parried) currentSFX = BASS_SampleLoad(false, Game::instance->audios[0].c_str(), 0, 0, 3, 0);
							else currentSFX = BASS_SampleLoad(false, Game::instance->audios[3].c_str(), 0, 0, 3, 0);
							if (currentSFX == 0) {
								cout << "Error audio not found!" << endl;
							}
							curr_SFX_channel = BASS_SampleGetChannel(currentSFX, false);

							break;
						case LEFT:
							if (parried) currentSFX = BASS_SampleLoad(false, Game::instance->audios[1].c_str(), 0, 0, 3, 0);
							else currentSFX = BASS_SampleLoad(false, Game::instance->audios[4].c_str(), 0, 0, 3, 0);
							if (currentSFX == 0) {
								cout << "Error audio not found!" << endl;
							}
							curr_SFX_channel = BASS_SampleGetChannel(currentSFX, false);
							break;
						case RIGHT:
							if (parried) currentSFX = BASS_SampleLoad(false, Game::instance->audios[2].c_str(), 0, 0, 3, 0);
							else currentSFX = BASS_SampleLoad(false, Game::instance->audios[5].c_str(), 0, 0, 3, 0);
							if (currentSFX == 0) {
								cout << "Error audio not found!" << endl;
							}
							curr_SFX_channel = BASS_SampleGetChannel(currentSFX, false);
							break;

						}
						if (!parried) { //If you miss the defence
							stats.missing_hp += 1 - (0.4f + stats.resistance); //Lose hp
						}
						else {
							weapon.defType = NONE;
						}
						cooldown = 0.0f;
						BASS_ChannelPlay(curr_SFX_channel, false); //Plays the audio
						parried = false;
					}
					weapon.attacked = false;
				}
				else {
					currentEnemy->enemyEntity->destroy(); //When killing the enemy, destroy the entity
					for (size_t i = 0; i < enemies.size(); i++)
					{
						if (enemies[i] == currentEnemy->enemyEntity) { //Erase it from the array
							enemies.erase(enemies.begin() + i);
						}
					}
					currentEnemy = NULL;
					isBattle = false; //End battle
				}

				if (enemies.size() <= 0) { //If all enemies have been defeated
					mapSwap = true;
					previous_stage = Stage_ID;
					if (obj != TUTORIAL) obj = LVLUP; 
					list.third = true;
					Stage_ID = MAP;//Go back to map

				}
			}
			else { //You can move if you are not in battle
				Matrix44 playerRotation;
				playerRotation.rotate(player->yaw * DEG2RAD, Vector3(0, 1, 0));
				Vector3 playerVel;
				Vector3 forward = playerRotation.rotateVector(Vector3(0, 0, -1));
				Vector3 right = playerRotation.rotateVector(Vector3(1, 0, 0));

#ifdef _DEBUG
				if (Input::isKeyPressed(SDL_SCANCODE_LSHIFT)) playerSpeed *= 2.0f;
#endif
				if (Input::isKeyPressed(SDL_SCANCODE_W)) playerVel = playerVel - (forward * playerSpeed);
				if (Input::isKeyPressed(SDL_SCANCODE_S)) playerVel = playerVel + (forward * playerSpeed);
				if (Input::isKeyPressed(SDL_SCANCODE_A)) playerVel = playerVel + (right * playerSpeed);
				if (Input::isKeyPressed(SDL_SCANCODE_D)) playerVel = playerVel - (right * playerSpeed);
				weapon.movementMotion = playerVel.x != 0 || playerVel.y != 0 || playerVel.z != 0;


				Vector3 nexPos = player->pos + playerVel;

				COL_RETURN ret1; //Colision to the boundaries of the arena
				for (size_t i = 0; i < entitiesColision.size(); i++)
				{
					Matrix44 cubeModel = Matrix44();
					cubeModel.translate(entitiesColision[i]->pos.x, entitiesColision[i]->pos.y, entitiesColision[i]->pos.z);
					cubeModel.scale(1.5f, 10.0f, 1.5f);
					entitiesColision[i]->model = cubeModel;
					ret1 = CheckColision(player->pos, nexPos, entitiesColision[i], elapsed_time,0.5f);
					if (ret1.colision) {
						nexPos = ret1.modifiedPosition;
						break;
					}
				}
				player->pos = nexPos;
				

				COL_RETURN ret; //Colision to the enemies
				for (size_t i = 0; i < enemies.size(); i++)
				{
					ret = CheckColision(player->pos, nexPos, enemies[i], elapsed_time);
					if (ret.colision) {
						nexPos = ret.modifiedPosition;
						isBattle = true;
						currentEnemy = new EnemyAI(2 + diff, enemies[i], 1 + diff);
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
		if (stats.missing_hp >= 1.0f) { //Death 
			//Init the game over
			Game::instance->gameOver->cam = Game::instance->camera;
			Game::instance->gameOver->entities = entities;
			Game::instance->gameOver->enemies = enemies;
			Game::instance->gameOver->Stage_ID = Stage_ID;
			Game::instance->gameOver->shader = shader;
			Game::instance->gameOver->gui_shader = gui_shader;
			Game::instance->gameOver->sky = sky;
			Game::instance->gameOver->terrain = terrain;
			stats.missing_hp = 0.0f;
			Game::instance->scene = GAMEOVER; //Change scene
		}
	}
#pragma endregion PLAYER_MOVEMENT

#pragma region CAMERA_MOVEMENT
	if (lvlUpMenu) { //If you are not leveling up, cursor is shown
		SDL_ShowCursor(true);
	}
	else { //Show cursor also on interaction or menu
		if (interaction || menu) SDL_ShowCursor(true);
		else SDL_ShowCursor(false);
	
		if (menu != !interaction) { //And if neither of those, move the camera with the centered mouse
			player->yaw += -Input::mouse_delta.x * 5.0f * elapsed_time;
			if (!toggle) Input::centerMouse();
			else SDL_ShowCursor(true);
		}

	}
#ifdef _DEBUG
	if (Input::wasKeyPressed(SDL_SCANCODE_8)) toggle = !toggle;
#endif
#pragma endregion CAMERA_MOVEMENT

#pragma region WEAPON_MOVEMENT
	float weapon_speed = 70.0f;
	if (weapon.attack) //Weapon animations
	{
		if (weapon.down) //Moving down
			weapon.attackMotion -= 5.0f * elapsed_time * weapon_speed;
		else //moving up
			weapon.attackMotion += 5.0f * elapsed_time * weapon_speed;

		if (weapon.attackMotion <= -90.0f) //check if up or down
		{
			weapon.down = false;
		}
		if (weapon.attackMotion >= 0.0f)
		{
			weapon.attack = false;
			weapon.attackMotion = 0.0f;
		}
	}
	//Defence animations
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
#pragma endregion WEAPON_MOVEMENT
#pragma region ENEMY_BEHAVIOUR
	if (!mapSwap) {
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
			if (!menu) {
				//Check if interaction
				if (!interaction && Game::instance->wasLeftButtonPressed) {
					interaction = CheckRayWithBarman(Game::instance->camera, barTender);
					nextText = 0;
				}
				else if (Game::instance->wasLeftButtonPressed) {
					nextText++;
				}
			}
		}
#pragma endregion BARTENDER_BEHAVIOUR

	}
	if(!mapSwap) previous_stage = Stage_ID;
#ifdef _DEBUG
	if (Input::wasKeyPressed(SDL_SCANCODE_COMMA)) {
		enemies.clear();
		if (lvl % 3 == 0)//Depending on the current battle
			LoadEnemiesInScene("data/enemies1.scene", &enemies);
		else if (lvl % 3 == 1) 
			LoadEnemiesInScene("data/enemies2.scene", &enemies);
		else
			LoadEnemiesInScene("data/enemies3.scene", &enemies);
	}
#endif
}

//GameOverStage

void GameOverStage::render()
{
	//set the clear color (the background color)
	SDL_ShowCursor(true);
	// Clear the window and the depth buffer
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	glDisable(GL_DEPTH_TEST);
	sky->render(); //Render the sky
	glEnable(GL_DEPTH_TEST);

	if (!entities.empty()) //Render the entities
	{
		for (size_t i = 0; i < entities.size(); i++) {
			RenderMesh(entities[i]->model, entities[i]->mesh, entities[i]->texture, shader, cam);
		}
	}
	RenderPlane(terrain->model, terrain->mesh, terrain->texture, terrain->shader, cam, tiling);

	if (!enemies.empty()) //Render the enemies
	{
		for (size_t i = 0; i < enemies.size(); i++)
		{
			Matrix44 enemyModel = Matrix44();
			enemyModel.translate(enemies[i]->pos.x, enemies[i]->pos.y, enemies[i]->pos.z);
			enemyModel.scale(enemies[i]->scale, enemies[i]->scale, enemies[i]->scale);
			enemyModel.rotate(enemies[i]->yaw * DEG2RAD, Vector3(0, 1, 0));
			enemies[i]->model = enemyModel;
			RenderMesh(enemyModel, enemies[i]->mesh, enemies[i]->texture, shader, cam); //"T-Pose mesh"
		}
	}

	//GUI RENDER
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	float winWidth, winHeight; //Render the gui
	winWidth = Game::instance->window_width;
	winHeight = Game::instance->window_width;
	float halfWidth = winWidth * 0.5f;
	float halfHeight = winHeight * 0.5f;
	RenderGUI(halfWidth, halfHeight, winWidth, winHeight, gui_shader, Texture::Get("data/gray_background.png"), Vector4(1,1,1,1));
	RenderGUI(halfWidth, 191.7, winWidth, 100, gui_shader, Texture::Get("data/black_bar.png"), Vector4(1, 1, 1, 1));
	if (RenderButton(395, 445, 250, 75, shader, Texture::Get("data/iconTextures/Back Button.png"), Vector4(1, 1, 1, 1), Texture::Get("data/iconTextures/Back  hover.png"))) {
		Game::instance->InitGameStage();
		Game::instance->intro->cam->lookAt(Vector3(148.92f, 77.76f, 57.58f), Vector3(30.0f, 21.99f, 9.88f), Vector3(0, 1, 0));
		Game::instance->scene = INTRO;
	}
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	drawText(190.0, 152.15,"YOU DIED!",Vector3(1,0,0),10.0f); //You died text
}
//No need to update anything
void GameOverStage::update(float elapsed_time)
{
}