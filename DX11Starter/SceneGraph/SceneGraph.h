#pragma once

#include "StaticObject.h"
#include "TemporalEntity.h"
#include "ColliderManager.h"
#include "KeyFrameData.h"
#include "ServerManager.h"

class SceneGraph
{
	// Player Buffer
	TemporalEntity* m_entities;

	// Static Object Buffer
	StaticObject* m_statics;

	// Number of entities
	int m_maxEntities;
	int m_entityCount;

	// Number of StaticObjects
	int m_staticObjectCount;

public:
	SceneGraph();
	~SceneGraph();

	void Init(int maxEntities, int causalityPerEntity);
	void Init(StaticObject* staticObjs, int staticobjectCount);

	// Use keyframe data to modify an entity by adding actions to the top of its stack
	void StackKeyFrame(KeyFrameData phantom);
	// Modifies the transform to avoid collisions
	bool PreventCollision(int entityId, Transform& position);

	void GetStatics(StaticObject** objs, int& count);

	TemporalEntity* GetEntity(int index);
	int GetEntityCount() const;

	int AddEntity(int maxImages, int maxPhenomina);

	Colliders2D::ColliderHandle GetColliderHandle(Colliders2D::ColliderType cType, float a = 0, float b = 0);

	// Networking
	void ClientRecieve(Buffer& data);
	void HostRecieve(Buffer& data, int playerId, ServerManager* server);


};
