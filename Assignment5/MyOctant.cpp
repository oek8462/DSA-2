#include "MyOctant.h"
using namespace Simplex;

uint MyOctant::m_uOctantCount = 0;
uint MyOctant::m_uMaxLevel = 3;
uint MyOctant::m_uIdealEntityCount = 5;
 
void MyOctant::Init(void) 
{
	m_uChildren = 0;

	m_fSize = 0.0f;
	m_uID = m_uOctantCount;
	m_v3Center = vector3(0.0f);
	m_v3Min = vector3(0.0f);
	m_v3Max = vector3(0.0f);

	m_pEntityMngr = MyEntityManager::GetInstance();
	m_pMeshMngr = MeshManager::GetInstance();

	for (int i = 0; i < 8; i++)
	{
		m_pChild[i] = nullptr;
	}
	m_pRoot = nullptr;
	m_pParent = nullptr;

}
MyOctant::MyOctant(uint a_nMaxLevel, uint a_nIdealEntityCount)
{
	Init();

	m_uMaxLevel = a_nMaxLevel;
	m_uIdealEntityCount = a_nIdealEntityCount;
	m_uOctantCount = 0;
	m_uID = m_uOctantCount;
	m_pRoot = this;
	m_lChild.clear();
	std::vector<vector3> minMax;
	uint objects = m_pEntityMngr->GetEntityCount();
	
	for (int i = 0; i < objects; i++)
	{
		MyEntity* entity = m_pEntityMngr->GetEntity(i);
		MyRigidBody* rigidBody = entity->GetRigidBody();
		minMax.push_back(rigidBody->GetMinGlobal());
		minMax.push_back(rigidBody->GetMaxGlobal());
	}
	MyRigidBody* rigidBody = new MyRigidBody(minMax);
	vector3 m_v3HalfWidth = rigidBody->GetHalfWidth();
	float max = m_v3HalfWidth.x;
	for (int i = 0; i < 3; i++)
	{
		if (max < m_v3HalfWidth[i]) 
		{
			max = m_v3HalfWidth[i];
		}
	}

	m_v3Center = rigidBody->GetCenterLocal();
	minMax.clear();
	delete rigidBody;
	rigidBody = nullptr;
	m_fSize = max * 2.0f;
	m_v3Min = m_v3Center - (vector3(max));
	m_v3Max = m_v3Center + (vector3(max));
	m_uOctantCount++;

	ConstructTree(m_uMaxLevel);
}
MyOctant::MyOctant(vector3 a_v3Center, float a_fSize) 
{
	Init();
	m_v3Center = a_v3Center;
	m_fSize = a_fSize;
	m_v3Min = m_v3Center - (vector3(m_fSize) / 2.0f);
	m_v3Max = m_v3Center + (vector3(m_fSize) / 2.0f);

	m_uOctantCount++;
}
MyOctant::MyOctant(MyOctant const& other)
{
	m_uChildren = other.m_uChildren;
	m_v3Center = other.m_v3Center;
	m_v3Min = other.m_v3Min;
	m_v3Max = other.m_v3Max;
	m_fSize = other.m_fSize;
	m_uID = other.m_uID;
	m_uLevel = other.m_uLevel;
	m_pParent = other.m_pParent;
	m_pRoot = other.m_pRoot;
	m_lChild = other.m_lChild;
	m_pMeshMngr = MeshManager::GetInstance();
	m_pEntityMngr = MyEntityManager::GetInstance();
	
}
MyOctant& MyOctant::operator=(MyOctant const& other)
{
	if (this != &other)
	{
		Release();
		Init();
		MyOctant temp(other);
		Swap(temp);
	}
	return *this;
}
MyOctant::~MyOctant(void) 
{
	Release();
}
void MyOctant::Swap(MyOctant& other) 
{
	std::swap(m_uChildren, other.m_uChildren);
	std::swap(m_fSize, other.m_fSize);
	std::swap(m_uID, other.m_uID);
	std::swap(m_pRoot, other.m_pRoot);
	std::swap(m_lChild, other.m_lChild);
	std::swap(m_v3Center, other.m_v3Center);
	std::swap(m_v3Min, other.m_v3Min);
	std::swap(m_v3Max, other.m_v3Max);
	
	m_pMeshMngr = MeshManager::GetInstance();
	m_pEntityMngr = MyEntityManager::GetInstance();
	for (int i = 0; i < 8; i++)
	{
		std::swap(m_pChild[i], other.m_pChild[i]);
	}
}
float MyOctant::GetSize(void)
{
	return m_fSize;
}
vector3 MyOctant::GetCenterGlobal(void) 
{
	return m_v3Center;
}
vector3 MyOctant::GetMinGlobal(void) 
{
	return m_v3Min;
}
vector3 MyOctant::GetMaxGlobal(void) 
{
	return m_v3Max;
}
bool MyOctant::IsColliding(uint a_uRBIndex) 
{
	int objectCount = m_pEntityMngr->GetEntityCount();
	//if index is larger than elements return false
	if (a_uRBIndex >= objectCount) 
	{
		return false;
	}

	//get entities and rigidbodies of blocks
	MyEntity* entity = m_pEntityMngr->GetEntity(a_uRBIndex);
	MyRigidBody* rigidBody = entity->GetRigidBody();
	vector3 min = rigidBody->GetMinGlobal();
	vector3 max = rigidBody->GetMaxGlobal();

	//x
	if (m_v3Max.x < min.x) 
	{
		return false;
	}
	if (m_v3Min.x > max.x)
	{
		return false;
	}

	//y
	if (m_v3Max.y < min.y)
	{
		return false;
	}
	if (m_v3Min.y > max.y)
	{
		return false;
	}

	//z
	if (m_v3Max.z < min.z)
	{
		return false;
	}
	if (m_v3Min.z > max.z)
	{
		return false;
	}

	return true;
}
void MyOctant::Display(uint a_nIndex, vector3 a_v3Color) 
{
	//make vector
	std::vector<MyOctant*> childrenVec;
	for (int i = 0; i < m_uChildren; i++) 
	{
		childrenVec.push_back(m_pChild[i]);
	}
	
	if (m_uID == a_nIndex) 
	{
		m_pMeshMngr->AddWireCubeToRenderList(glm::translate(IDENTITY_M4, m_v3Center) * glm::scale(vector3(m_fSize)), a_v3Color, RENDER_WIRE);
		return;
	}
	//loop through vector and display
	for (int i = 0; i < childrenVec.size(); i++)
	{
		childrenVec[i]->Display(a_nIndex);
	}
	
}
void MyOctant::Display(vector3 a_v3Color) 
{
	//make vector
	std::vector<MyOctant*> childrenVec;
	for (int i = 0; i < m_uChildren; i++)
	{
		childrenVec.push_back(m_pChild[i]);
	}
	//loop through vector and display
	for (int i = 0; i < childrenVec.size(); i++)
	{
		childrenVec[i]->Display(a_v3Color);
	}
	m_pMeshMngr->AddWireCubeToRenderList(glm::translate(IDENTITY_M4, m_v3Center) * glm::scale(vector3(m_fSize)), a_v3Color, RENDER_WIRE);
}
void MyOctant::DisplayLeafs(vector3 a_v3Color) 
{
	for (int i = 0; i < m_lChild.size(); i++)
	{
		m_lChild[i]->DisplayLeafs(a_v3Color);
	}
	m_pMeshMngr->AddWireCubeToRenderList(glm::translate(IDENTITY_M4, m_v3Center) * glm::scale(vector3(m_fSize)), a_v3Color, RENDER_WIRE);
}
void MyOctant::ClearEntityList(void) 
{
	//clear children of lists 
	for (int i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->ClearEntityList();
	}
	//clear entitylist itself
	m_EntityList.clear();
}
void MyOctant::Subdivide(void) 
{
	if (m_uLevel >= m_uMaxLevel) //if at maximum depth return with no changes
	{
		return;
	}

	if (m_uChildren != 0)  //if node is already subdivided return with no changes
	{
		return;
	}

	m_uChildren = 8;

	float size = m_fSize / 4.0f;
	float sizeD = m_fSize * 2.0f;

	vector3 center;
	//positions are defined before each child octant added
	center.x = m_v3Center.x - size;
	center.y = m_v3Center.y - size;
	center.z = m_v3Center.z - size;

	m_pChild[0] = new MyOctant(center, m_fSize / 2); //bottom left back

	center.x = m_v3Center.x + size;
	center.y = m_v3Center.y - size;
	center.z = m_v3Center.z - size; //bottom right back
	m_pChild[1] = new MyOctant(center, m_fSize / 2);

	center.x = m_v3Center.x + size;
	center.y = m_v3Center.y - size;
	center.z = m_v3Center.z + size; //bottom R front
	m_pChild[2] = new MyOctant(center, m_fSize / 2);

	center.x = m_v3Center.x - size;
	center.y = m_v3Center.y - size;
	center.z = m_v3Center.z + size;//bottom L Front
	m_pChild[3] = new MyOctant(center, m_fSize / 2);

	center.x = m_v3Center.x - size;
	center.y = m_v3Center.y + size;
	center.z = m_v3Center.z + size;//top L front
	m_pChild[4] = new MyOctant(center, m_fSize / 2);

	center.x = m_v3Center.x + size;
	center.y = m_v3Center.y + size;
	center.z = m_v3Center.z - size; //top R back
	m_pChild[5] = new MyOctant(center, m_fSize / 2);

	center.x = m_v3Center.x + size;
	center.y = m_v3Center.y + size;
	center.z = m_v3Center.z - size;//top R back
	m_pChild[6] = new MyOctant(center, m_fSize / 2);

	center.x = m_v3Center.x + size;
	center.y = m_v3Center.y + size;
	center.z = m_v3Center.z + size;//top R front
	m_pChild[7] = new MyOctant(center, m_fSize / 2);

	for (int i = 0; i < 8; i++)
	{
		m_pChild[i]->m_pRoot = m_pRoot;
		m_pChild[i]->m_pParent = this;
		m_pChild[i]->m_uLevel = m_uLevel + 1;
		if (m_pChild[i]->ContainsMoreThan(m_uIdealEntityCount))
		{
			m_pChild[i]->Subdivide();
		}
	}
}
MyOctant* MyOctant::GetChild(uint a_nChild) 
{
	//if child out of range, return null
	if (a_nChild > 7) 
	{
		return nullptr;
	}
	//return child id
	return m_pChild[a_nChild];
}
MyOctant* MyOctant::GetParent(void) 
{
	return m_pParent;
}
bool MyOctant::IsLeaf(void) 
{
	return m_uChildren == 0;
}
bool MyOctant::ContainsMoreThan(uint a_nEntities) 
{
	int count = 0;

	for (int i = 0; i < m_pEntityMngr->GetEntityCount(); i++)
	{
		//if colliding, add to count
		if (IsColliding(i)) 
		{
			count++;
		}
		//if the count exceeds optimal count, cell contains enough to be subdivided
		if (count > a_nEntities)
		{
			return true;
		}
	}
	return false;
}
void MyOctant::KillBranches(void) 
{
	//loop through and delete branches
	for (int i = 0; i < m_uChildren; i++)
	{
		delete m_pChild[i];
		m_pChild[i] = nullptr;
	}
	//set child count to zero
	m_uChildren = 0;
}
void MyOctant::ConstructTree(uint a_nMaxLevel)
{
	//if level is zero
	if (m_uLevel == 0) 
	{
		//initialize tree, clear data 
		m_uMaxLevel = a_nMaxLevel;
		m_uOctantCount = 1;

		m_EntityList.clear();

		KillBranches();
		m_lChild.clear();

		//if there are more entities than the ideal count, subdivide
		if (ContainsMoreThan(m_uIdealEntityCount))
		{
			Subdivide();
		}

		//assign Ids to all entities 
		AssignIDtoEntity();

		//make the list
		ConstructList();
	}
}
void MyOctant::AssignIDtoEntity(void) 
{
	//call assign id on all children
	for (int i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->AssignIDtoEntity();
	}
	if (m_uChildren == 0) 
	{
		for (int i = 0; i < m_pEntityMngr->GetEntityCount(); i++)
		{
			//if colliding
			if (IsColliding(i)) 
			{
				//push to entity list
				m_EntityList.push_back(i);
				m_pEntityMngr->AddDimension(i, m_uID);
			}
		}
	}

}
uint MyOctant::GetOctantCount(void) 
{
	return m_uOctantCount;
}
void MyOctant::ConstructList(void) 
{
	for (int i = 0; i < m_uChildren; i++)
	{
		if (m_EntityList.size() > 0)
		{
			m_pRoot->m_lChild.push_back(this);
		}
	}
}

void MyOctant::Release(void)
{
	if (m_uLevel == 0)
	{
		KillBranches();
	}
	m_uChildren = 0;
	m_fSize = 0.0f;
	m_EntityList.clear();
	m_lChild.clear();
}


	
