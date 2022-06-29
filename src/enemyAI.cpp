#include "enemyAI.h"


EnemyAI::EnemyAI(int s, EntityMesh* ent, int h)
{
	attacks.reserve(s); //Generates the size for the number of attacks
	enemyEntity = ent; //Adds the entity 
	hp = h; //And the hp it has
}

void EnemyAI::GenerateAttacks()
{
	/*
	Randomize attacks
	*/
	for (size_t i = 0; i < attacks.capacity(); i++) //Fills up the array with random attacks
	{
		int randomNumber = (int) floor(random() * 3.0f);
		attacks.push_back((POSITION)randomNumber);
	}
}


void EnemyAI::ClearAttacks()
{
	attacks.clear(); //Clears the attacks of the enemy
}

bool EnemyAI::EmptyAttacks()
{
	return attacks.empty(); //Checks if the attacks from the enemy are empty
}

void EnemyAI::ChangeMaxAttacks(int number)
{ 
	attacks.clear(); //Resizes the number of attacks
	attacks.resize(number);
}

POSITION EnemyAI::GetNextAttack()
{
	/*
	Randomize next attack
	*/
	int randomNumber = (int)floor(random() * attacks.size()); //Gets the next attacks in a random manner
	POSITION attack = attacks[randomNumber];
	attacks.erase(attacks.begin() + randomNumber);
	return attack;
}
