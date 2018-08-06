#include "StdAfx.h"
#include "SPH.h"
#define PI 3.14159265358979323846264338327950L

Particle::Particle()
{
	m_Vel.Set(0,0,0);
	m_VelEval.Set(0,0,0);
	m_Acc.Set(0,0,0);
}

Particle::~Particle()
{

}

CSPH::CSPH(void)
{
	m_smoothLen = 0.01;
	m_timeStep = 0.001;
	m_curStep = 0;
	m_mass = 0.00020543f;
	m_K = 1.5f;
	m_Kpoly6 = 315 * m_mass/((64*PI)*pow(m_smoothLen,9));
	m_Kspiky = 45.0f/(PI*pow(m_smoothLen,6));
	m_Kviscosity = 0.2f;

	m_GlassRange[0] = -0.05;
	m_GlassRange[1] = 0.05;
	m_GlassRange[2] = 0;
	m_GlassRange[3] = 1;
	m_GlassRange[4] = -0.05;
	m_GlassRange[5] = 0.05;

	m_GlassNormal[0].Set(1,0,0);
	m_GlassNormal[1].Set(-1,0,0);
	m_GlassNormal[2].Set(0,1,0);
	m_GlassNormal[3].Set(0,-1,0);
	m_GlassNormal[4].Set(0,0,1);
	m_GlassNormal[5].Set(0,0,-1);

	m_gravity.Set(0,-9.8,0);
	m_iMoveGlassDir = 0;

	ballPos[0] = 0;
	ballPos[1] = 0;
	ballPos[2] = 0;
	ballPos[3] = 0;
	ballPos[4] = 0;
	ballPos[5] = 0;

	fopen_s(&file,"force.txt","w");
	countNum3 = 0;
}


CSPH::~CSPH(void)
{
	if (m_particle)
	{
		delete[] m_particle;
	}

	//手控器
	fclose(file);
}

//初始化
void CSPH::Init(int particleNum)
{
	m_particleNum = particleNum;
	m_particle = new Particle[m_particleNum];
	InitPos();
}

//初始化例子的位置
void CSPH::InitPos()
{
	float dis = m_smoothLen/**0.6*/; //粒子之间的距离取光滑半径的0.6，使得刚开始就能作用的上
	int cx = int(powf(m_particleNum/4,1.0/3)); 
	int cy = 4*cx;
	int cz = cx;
	for (int i = 0; i < cy; i++)
	{
		for (int j = 0; j < cx; j++)
		{
			for (int k = 0; k < cz; k++)
			{					
				int index = i*cx*cz + j*cz + k;
				if(index>=m_particleNum) 
					return;
				float y = i * dis/* * 0.6*/ + 0.1;
				float x = (j - cx*0.5) * dis;
				float z = (k - cz*0.5) * dis;
				m_particle[index].m_pos.Set(x,y,z);  //初始化例子的位置		
			}
		}
	}
}

//求解边界，保存在m_boundMin,m_boundMax中
void CSPH::FindBound()
{
	float minX,minY,minZ;
	float maxX,maxY,maxZ;
	minX = minY = minZ = 10000;
	maxX = maxY = maxZ = -10000;

	//遍历所以例子的位置，求出最小的x，y，z
	for (int i = 0; i < m_particleNum; ++i)
	{
		if (m_particle[i].m_pos.x < minX)
		{
			minX = m_particle[i].m_pos.x;
		}
		if (m_particle[i].m_pos.x > maxX)
		{
			maxX = m_particle[i].m_pos.x;
		}

		if (m_particle[i].m_pos.y < minY)
		{
			minY = m_particle[i].m_pos.y;
		}
		if (m_particle[i].m_pos.y > maxY)
		{
			maxY = m_particle[i].m_pos.y;
		}

		if (m_particle[i].m_pos.z < minZ)
		{
			minZ = m_particle[i].m_pos.z;
		}
		if (m_particle[i].m_pos.z > maxZ)
		{
			maxZ = m_particle[i].m_pos.z;
		}
	}
	m_boundMin.Set(minX,minY,minZ);
	m_boundMax.Set(maxX,maxY,maxZ);
}

void CSPH::FindNeighbour()
{
	//计算网格的个数
	FindBound();
	m_gridNumX = (m_boundMax.x - m_boundMin.x + m_smoothLen)/m_smoothLen;
	m_gridNumY = (m_boundMax.y - m_boundMin.y + m_smoothLen)/m_smoothLen;
	m_gridNumZ = (m_boundMax.z - m_boundMin.z + m_smoothLen)/m_smoothLen;
	m_gridNum = m_gridNumX*m_gridNumY*m_gridNumZ;
	const int gridSaveNum = 100; //每个网格中存放的粒子数
	//为网格分配内存，网格中保存particle的索引

	Grid = new int[m_gridNum*gridSaveNum];
	GridNum = new int[m_gridNum];
	memset(Grid,0,sizeof(int)*m_gridNum*gridSaveNum);
	memset(GridNum,0,sizeof(int)*m_gridNum);


	for (int i = 0; i < m_particleNum; ++i)
	{
		Vector3DF gridIndexTemp = m_particle[i].m_pos - m_boundMin; //当前粒子与边界的差值
		int cx = int(gridIndexTemp.x/m_smoothLen);
		int cy = int(gridIndexTemp.y/m_smoothLen);
		int cz = int(gridIndexTemp.z/m_smoothLen);
		int gridIndex = cx*m_gridNumY*m_gridNumZ + cy*m_gridNumZ + cz;//当前粒子从属的网格
		Grid[gridIndex*gridSaveNum + GridNum[gridIndex]] = i;             //该网格记录例子索引
		m_particleIndexInGrid[i] = gridIndex;
		GridNum[gridIndex]++;                                         //记录该网格已经存在了多少例子
		if (GridNum[gridIndex] > 100)
		{
			AfxMessageBox(_T("邻居过多！"));
			return;
		}
	}
	//查找邻居
	memset(m_particleNeighbour,0,sizeof(int)*PARTICLE_NUM*200);
	memset(m_particleNeighbourNum,0,sizeof(int)*PARTICLE_NUM);

	for (int i = 0; i < m_particleNum; ++i)
	{
		Vector3DF gridIndexTemp = m_particle[i].m_pos - m_boundMin; //当前粒子与边界的差值
		int cx = int(gridIndexTemp.x/m_smoothLen);
		int cy = int(gridIndexTemp.y/m_smoothLen);
		int cz = int(gridIndexTemp.z/m_smoothLen);
		//遍历上下左右各27个网格单元
		for (int ix = cx - 1; ix <= cx + 1; ix++)
		{
			for (int iy = cy - 1; iy <= cy + 1; iy++)
			{
				for (int iz = cz - 1; iz <= cz + 1; iz++)
				{
					int gridIndex = ix*m_gridNumY*m_gridNumZ + iy*m_gridNumZ + iz;//当前粒子从属的网格
					if(gridIndex < 0 || gridIndex > m_gridNum)
						continue;
					for (int j = 0; j < GridNum[gridIndex]; ++j)
					{
						int particleId = Grid[gridIndex*gridSaveNum + j]; //粒子的索引id
						if (particleId == i) //如果是该粒子本身
							continue;
						float disToNeighbour = m_particle[i].m_pos.DistanceTo(m_particle[particleId].m_pos); //该粒子与相邻粒子的距离
						if (disToNeighbour <= m_smoothLen)
						{
							m_particleNeighbour[i][m_particleNeighbourNum[i]] = particleId;
							m_particleNeighbourDistance[i][m_particleNeighbourNum[i]] = disToNeighbour;
							m_particleNeighbourNum[i]++;
						}
					}
				}
			}
		}
	}
	if (Grid)
	{ 
		delete[] Grid;
	}
	if(GridNum)
	{
		delete[] GridNum;
	}
	
}

//计算每个粒子的密度、压力
void CSPH::ComputeDensity()
{
	//计算密度公式 315/(64*PAI*h^9)Sigma(h^2-r^2)^3	
	//这里把315/(64*PAI*h^9)定义为常量。
	float h6 = pow(m_smoothLen,6);
	
	for (int i = 0; i < m_particleNum; ++i)
	{
		m_particle[i].m_density = 0;
	}

	//
	for (int i = 0; i < m_particleNum; ++i)
	{
		float sum = h6; //先把自己的密度加上
		for (int j = 0; j < m_particleNeighbourNum[i]; ++j)
		{
			float h2_r2 = m_smoothLen*m_smoothLen - m_particleNeighbourDistance[i][j]*m_particleNeighbourDistance[i][j];
			sum += pow(h2_r2,3);
		}

		m_particle[i].m_density = sum * m_Kpoly6;
		m_particle[i].m_pressure = m_K*(m_particle[i].m_density - 1000.0f); //压力=K*(ρ-ρ0）
	}
}

void CSPH::ComputeFroce()
{
	for (int i = 0; i < m_particleNum; i++)
	{
		m_particle[i].m_Acc.Set(0,0,0);	
	}
	float h = m_smoothLen;
	Vector3DF vdiff;
	Vector3DF diff;
	
	Vector3DF force1(0,0,0); //压力
	Vector3DF force2(0,0,0); //粘滞力

	for (int i = 0; i < m_particleNum; ++i)
	{
		Vector3DF force(0,0,0);
		for (int j = 0; j < m_particleNeighbourNum[i]; ++j)
		{
			float r = m_particleNeighbourDistance[i][j];
			float r_h = h-r; 
			int otherParticleId = m_particleNeighbour[i][j];
			diff = m_particle[i].m_pos - m_particle[otherParticleId].m_pos; //位置的微分
			vdiff = m_particle[otherParticleId].m_Vel - m_particle[i].m_Vel; //速度的微分
			//压力
			float param = (m_particle[i].m_pressure + m_particle[otherParticleId].m_pressure)*0.5*m_Kspiky*r_h*r_h/ r;
			force1 =diff * (m_mass * param / (m_particle[i].m_density * m_particle[otherParticleId].m_density)); 
			//粘滞力
			force2 = vdiff * (m_mass * r_h  * m_Kviscosity * m_Kspiky / (m_particle[i].m_density * m_particle[otherParticleId].m_density));

			force += force1;
			force += force2;
		}	
		m_particle[i].m_Acc = force;
	}

}

void CSPH::DetectCollision(float timeStep)
{
	Vector3DF prePos;
	Vector3DF colAcc;
	for (int i = 0; i < m_particleNum; ++i)
	{
		colAcc.Set(0,0,0);
		prePos = m_particle[i].m_pos + m_particle[i].m_VelEval * timeStep; //计算当前时刻粒子的位置，如果没有碰撞应该走到这里
		GlassCollision(prePos,colAcc,m_particle[i].m_Vel);
		m_particle[i].m_Acc += colAcc;
	}
}


void CSPH::GlassCollision(Vector3DF &prePos, Vector3DF &colAcc, Vector3DF &vel)
{
	float radius = 0.004;
	float EPSILON = 0.00001f;
	float stiff = 30000;
	float damp = 128;
	float r = 0.010; //小球的半径

	for (int i = 0; i < 6; i++)
	{
		int j = i / 2;
		float diff;
		if(i%2) 
			diff = 2 * radius - (m_GlassRange[i]- prePos[j]);//1,3,5则运行这个，表示右边界。	如果越界了则右面括号是负值，diff就大于2*radius
		else 
			diff = 2 * radius - (prePos[j] - m_GlassRange[i]);	//0,2,4表示左边界。				如果越界了则右面括号是负值，diff就大于2*radius
		if (diff > EPSILON)//只要大于EPSILON就要执行，说明前面足够靠近边界就会进入。当前面重合边界时候应该已经大于等于2*radius了。
		{
			float v0 = m_GlassNormal[i]*vel;
			float reverse = stiff*diff - damp*v0;//把移动速度转换到方向向量上	,m_nor为各个碰撞面的法向量。把当前运行的速度转换到法向量上。当前看如果碰撞则这个值应该是<0的
			colAcc += m_GlassNormal[i] * reverse;//一个是距离因素一个是运动方向因素，都可以影响粒子沿着碰撞面的法向量运动。
		}
	}
}


void CSPH::UpdatePos(float timeStep)
{
	for (int i = 0; i < m_particleNum; i++)// 流体模拟    
	{
		Vector3DF finalAcc = m_particle[i].m_Acc + m_gravity;
		Vector3DF velEval = m_particle[i].m_VelEval + finalAcc*timeStep;
		m_particle[i].m_pos += velEval*timeStep;
		m_particle[i].m_Vel = (m_particle[i].m_VelEval + velEval)*0.5;
		m_particle[i].m_VelEval = velEval;
	}
	//MoveGlass();
}

//移动容器
void CSPH::MoveGlass()
{
	if (m_iMoveGlassDir == 0)//向右增加
	{
		if (m_GlassRange[5] < 0.2)
			m_GlassRange[5] += 0.0005;
		else
			m_iMoveGlassDir = 1;
	}
	else if (m_iMoveGlassDir == 1)
	{
		if (m_GlassRange[5] > 0.01)
			m_GlassRange[5] -= 0.0005;
		else m_iMoveGlassDir = 0;
	}
}

void CSPH::ComputeTouchFroce()
{

};



void CSPH::saveData()
{
	FILE *file;
	fopen_s(&file,"data.txt","a+");
	if (file == NULL)
	{
		return;
	}
	fprintf_s(file,"step:%d\n",m_curStep);
	for (int i = 0; i < 10; ++i)
	{
		for (int j = 0; j < 3; j++)
		{
			fprintf_s(file,"%.3d ",m_particle[i].m_Acc[j]);

		}
		fprintf_s(file, "\n");
	}
	fclose(file);
}


void CSPH::run(cOSG *osg)
{

	osg->SetSphereNum(m_particleNum);
	Sleep(500);
	theOsg = osg;
	runHandle = (HANDLE)_beginthreadex(NULL, 0, runThread, this, 0, NULL);
	
}
	

unsigned __stdcall CSPH::runThread(void *param)
{
	CSPH *sph = (CSPH*)param;
	//double pos[6] = {100,200,0,0,0,0};
	while(sph->m_curStep < 100000)
	{
		sph->FindNeighbour();
		sph->ComputeDensity();
		sph->ComputeFroce();
		sph->DetectCollision(sph->m_timeStep);
		sph->UpdatePos(sph->m_timeStep);
		sph->ComputeTouchFroce();
		Sleep(5);
		//Sleep(5);
		//Sleep(100);
		
		sph->m_curStep++;

		double color[3] = {255,255,255};
		double pos[3];

		for (int i = 0; i < sph->m_particleNum; ++i)
		{
			pos[0] = 1000*sph->m_particle[i].m_pos[0];
			pos[2] = 1000*sph->m_particle[i].m_pos[1];
			pos[1] = 1000*sph->m_particle[i].m_pos[2] * -1;
			sph->theOsg->SetSphereProperty(i, pos, color);
		}
		sph->theOsg->SetTextStep(sph->m_curStep);

		double posball[6];
		posball[0] = sph->ballPos[0];
		posball[2] = sph->ballPos[1];
		posball[1] = sph->ballPos[2] * -1;
		posball[3] = 0;
		posball[4] = 0;
		posball[5] = 0;
		sph->theOsg->SetStickMove(posball);
	}
	return 0;
}