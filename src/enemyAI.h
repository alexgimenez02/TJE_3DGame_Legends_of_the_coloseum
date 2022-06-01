#ifndef ENEMYAI_H
#define ENEMYAI_H

#include "utils.h"

enum POSITION
{
	LEFT = 0,
	UP = 1,
	RIGHT = 2
};
class EnemyAI
{
public:
	//atributes
	vector<POSITION> attacks;
	//ctor
	EnemyAI(int s);

	//methods
	void GenerateAttacks();
	void ClearAttacks();
	bool EmptyAttacks();
	void ChangeMaxAttacks(int number);
	POSITION GetNextAttack();
};
#endif //ENEMYAI_H
