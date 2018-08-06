#pragma once
#include "Vector3DF.h"
#include "MFC_OSG.h"

#define PARTICLE_NUM 4000


class Particle
{
public:
	Particle();
	~Particle();
	Vector3DF m_pos;   //位置
	double color[3];      //颜色

	float m_pressure;  //压力
	float m_density;   //密度

	Vector3DF m_Vel;   //速度
	Vector3DF m_VelEval;  //预估计速度
	Vector3DF m_Acc;   //加速度

};


class CSPH
{
public:
	CSPH(void);
	~CSPH(void);
	void Init(int particleNum); //初始化 指定粒子的个数、相的个数, 以及进行粒子位置的初始化
	void InitPos();				//初始化粒子的位置
	void FindBound();			//求解网格的边界，根据粒子的最大值、最小值求解
	void FindNeighbour();		//查找每一个例子的相邻例子

	void ComputeDensity();			//得到粒子的密度
	void ComputeFroce();			//计算粒子的受力
	void DetectCollision(float timeStep);			//碰撞检测
	void GlassCollision(Vector3DF &prePos, Vector3DF &colPos, Vector3DF &vel);			//碰撞检测
	
	void UpdatePos(float timeStep); //更新粒子的位置
	void MoveGlass();				//移动容器
	void ComputeTouchFroce();		//计算接触力

	void run(cOSG *osg); //运动

	void saveData(); //保存数据
	
	cOSG *theOsg;
	HANDLE runHandle;
	static unsigned __stdcall runThread(void *param);

	HANDLE sendHandle;
	static unsigned __stdcall sendThread(void *param);
	

	

	Particle *m_particle; //粒子
	int m_particleNum;

	float m_K;			//密度转换压力的系数
	float m_mass;       //例子质量
	float m_Kpoly6;     //密度核函数系数
	float m_Kspiky;     //压力核函数系数
	float m_Kviscosity; //粘度参数 0.2f

	Vector3DF m_gravity;  //重力

	//
	float m_smoothLen;//光滑核函数半径h
	float m_timeStep; //时间步长 使用了一个固定步长 0.001
	int m_curStep;	  //当前步数


	//网格
	Vector3DF m_boundMin; //下边界
	Vector3DF m_boundMax; //上边界
	int m_gridNumX,m_gridNumY,m_gridNumZ; //网格的大小，x*y*z; 根据粒子位置计算
	int m_gridNum;
	int *Grid; //代表网格中存放的粒子的索引，每一个网格最多存放一百个 new [x][y][z][100]
	int *GridNum;   //代表上面对应网格中存放粒子的个数 new [x][y][z]

	//
	int m_particleIndexInGrid[PARTICLE_NUM]; //粒子在网格中的位置
	int m_particleNeighbour[PARTICLE_NUM][100]; //例子邻居的id;
	int m_particleNeighbourNum[PARTICLE_NUM];   //例子邻居的数量;
	float m_particleNeighbourDistance[PARTICLE_NUM][100];  //例子邻居的距离

	//容器
	float m_GlassRange[6]; //x(-+) y(-+) z(-+)
	Vector3DF m_GlassNormal[6];  //六个面对应的法向量
	int m_iMoveGlassDir;
	double ballPos[6]; //小球的位置
	double ballPosPer[3];

	//
	FILE *file;
	int countNum3;
};

