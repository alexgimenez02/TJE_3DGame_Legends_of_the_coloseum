#ifndef ENEMYAI_H
#define ENEMYAI_H

#include "utils.h"
#include "entity.h"

class EnemyAI
{
public:
	//atributes
	vector<POSITION> attacks;
	EntityMesh* enemyEntity, *enemyWeapon;
	int hp;
	//ctor
	EnemyAI(int s, EntityMesh* ent, int h);

	//methods
	void GenerateAttacks();
	void ClearAttacks();
	bool EmptyAttacks();
	void ChangeMaxAttacks(int number);
	POSITION GetNextAttack();
};
#endif //ENEMYAI_H
