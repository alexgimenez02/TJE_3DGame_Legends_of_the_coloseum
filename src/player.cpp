#include "player.h"

Player::Player(Vector3 p, Vector3 e, Vector3 c, Vector3 col, Mesh* weapon_mesh, Texture* weapon_texture)
{
	pos = p;
	eye = e;
	center = c;
	col_center = col;
	weapon->mesh = weapon_mesh;
	weapon->texture = weapon_texture;
	
}

void Player::AttackAction()
{
	attack = true;
}
