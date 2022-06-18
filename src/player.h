#ifndef PLAYER_H
#define PLAYER_H

#include "utils.h"
#include "entity.h"
class Player
{
private:
	float attackMotion = 0.0f, defenceMotion = 0.0f, defenceMotionUp = 0.0f, defenceRotation = 0.0f;
	bool attack = false, down = true, movementMotion = false, defence = false, defMotion = false, defMotionUp = false, defRotation = false;
	POSITION defType = NONE;

public:
	Vector3 pos, eye, center, col_center;
	Vector3 cam_offset = Vector3(-0.05f, 0.25f, 2.6f);
	EntityMesh* weapon;

	Player(Vector3 p, Vector3 e, Vector3 c, Vector3 col, Mesh* weapon_mesh, Texture* weapon_texture);

};

#endif // !PLAYER_H
