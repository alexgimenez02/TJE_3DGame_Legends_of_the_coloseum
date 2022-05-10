#ifndef WORLD_H
#define WORLD_H

#include "entity.h"
#include "utils.h"

class World
{
public:
	EntityMap currentMap;
	vector<EntityMap> mapList;
	vector<EntityMesh> propList;

	World();
	~World();

	void loadCurrentMap(int index);
	Vector3 worldToCell(Vector3 worldPos);
	bool isValid(Vector3 worldPos);

};
#endif // !WORLD_H



