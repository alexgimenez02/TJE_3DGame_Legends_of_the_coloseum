#include "enemyAI.h"


EnemyAI::EnemyAI(int s, EntityMesh* ent, int h)
{
	attacks.reserve(s);
	enemyEntity = ent;
	hp = h;
}

void EnemyAI::GenerateAttacks()
{
	/*
	Randomize attacks
	*/
	for (size_t i = 0; i < attacks.capacity(); i++)
	{
		int randomNumber = (int) floor(random() * 3.0f);
		attacks.push_back((POSITION)randomNumber);
	}
}


void EnemyAI::ClearAttacks()
{
	attacks.clear();
}

bool EnemyAI::EmptyAttacks()
{
	return attacks.empty();
}

void EnemyAI::ChangeMaxAttacks(int number)
{
	attacks.clear();
	attacks.resize(number);
}

POSITION EnemyAI::GetNextAttack()
{
	/*
	Randomize next attack
	*/
	int randomNumber = (int)floor(random() * attacks.size());
	POSITION attack = attacks[randomNumber];
	attacks.erase(attacks.begin() + randomNumber);
	return attack;
}
