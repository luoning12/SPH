#pragma once

#include <osg/ShapeDrawable>
#include <osg/Material>
#include <osg/PositionAttitudeTransform>
#include <osg/MatrixTransform>
#include <osgText/Text>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/api/win32/GraphicsWindowWin32>
#include <osgGA/TrackballManipulator>
#include <osgGA/KeySwitchMatrixManipulator>
#include <osgDB/DatabasePager>
#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgUtil/Optimizer>
#include <string>
#include <vector>
#include <sstream>

struct MySphere
{
public:
	MySphere():m_IsUpdate(FALSE)
	{
		m_Node = new osg::PositionAttitudeTransform;
		m_Sphere =  new osg::Geode;
		osg::ref_ptr<osg::Sphere> sphere = new osg::Sphere(osg::Vec3(0,0,0), 1.2f);

		osg::ref_ptr<osg::TessellationHints> hints = new osg::TessellationHints;
		hints->setDetailRatio(0.5);
		osg::ref_ptr<osg::ShapeDrawable> shapeDrawable = new osg::ShapeDrawable(sphere.get(), hints);

		m_Sphere->addDrawable(shapeDrawable.get());
		m_Node->addChild(m_Sphere);
		m_Material = new osg::Material;
		m_Sphere->getOrCreateStateSet()->setAttribute(m_Material);

		m_Material->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4(1.0, 1.0, 1.0, 1.0));
		m_Material->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(1.0, 0.5, 0.5, 1.0));
		m_Material->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(1.0, 1.0, 1.0, 1.0));
		m_Material->setShininess(osg::Material::FRONT_AND_BACK, 60.0);
	};
	~MySphere(){};
public:
	void SetProperty(osg::Vec3 position, osg::Vec4 color)
	{
		if (m_IsUpdate == FALSE)
		{
			m_Position = position;
			m_Color = color;
			m_IsUpdate = TRUE;
		}
	};
	void UpdateProperty()
	{
		if (m_IsUpdate == TRUE)
		{
			m_Node->setPosition(m_Position);
			m_Material->setDiffuse(osg::Material::FRONT_AND_BACK, m_Color);
			m_IsUpdate = FALSE;
		}
	}
	osg::ref_ptr<osg::Node> GetNode()
	{
		return m_Node;
	}
private:
	//pointer
	osg::ref_ptr<osg::PositionAttitudeTransform> m_Node;
	osg::ref_ptr<osg::Geode> m_Sphere;
	osg::ref_ptr<osg::Material> m_Material;
	//property
	osg::Vec3 m_Position;
	osg::Vec4 m_Color;
	bool m_IsUpdate;
};




class cOSG
{
public:
    cOSG(HWND hWnd);
    ~cOSG();

public:
	void InitOSG();

	//设置场景的粒子数量，调用后需要Sleep一段时间，以等待渲染线程将粒子创建完毕
	bool SetSphereNum(int num);

	//设置粒子的位置和颜色，index为粒子索引，当索引超出场景的粒子总数时，函数返回false
	bool SetSphereProperty(int index, double position[3], double color[3]);

	//设置搅拌棍的位姿，姿态由Z-Y-X欧拉角描述，[x y z alpha beta gamma]
	bool SetStickMove(double position[6]);

	//设置文字：当前步数，总步数
	bool SetTextStep(int currentstep/*, int totalstep*/);

private:
	bool m_FlagSetSphereNum;
	bool m_FlagSetSphereProperty;
	bool m_FlagSetStickMove;
	bool m_FlagSetTextStep;

	int m_SphereNum;
	osg::Matrix m_StickMoveMatrix;
	std::string m_StepText;


public:
    void InitManipulators(void);
    void InitSceneGraph(void);
    void InitCameraConfig(void);
    void SetupWindow(void);
    void SetupCamera(void);
    void PreFrameUpdate(void);
    void PostFrameUpdate(void);
    void Done(bool value) { mDone = value; };
    bool Done(void) { return mDone; };

    osgViewer::Viewer* getViewer() { return mViewer; };

private:
	osg::Geode* CreateSphere();

private:
    bool mDone;
    HWND m_hWnd;
    osgViewer::Viewer* mViewer;
    osg::ref_ptr<osgGA::TrackballManipulator> trackball;
    osg::ref_ptr<osgGA::KeySwitchMatrixManipulator> keyswitchManipulator;
	osg::ref_ptr<osg::Camera> m_HUDCamera;
	osg::ref_ptr<osgText::Text> m_Text;

private:
	//scene node
	osg::ref_ptr<osg::Group> m_Root;
	osg::ref_ptr<osg::Node> m_Grid;
	osg::ref_ptr<osg::Group> m_SphereGroup;
	std::vector<MySphere> m_SphereVector;

	osg::ref_ptr<osg::Node> m_StickModel;
	osg::ref_ptr<osg::MatrixTransform> m_StickMove;

};




class CRenderingThread : public OpenThreads::Thread
{
public:
    CRenderingThread( cOSG* ptr );
    virtual ~CRenderingThread();

    virtual void run();

protected:
    cOSG* _ptr;
    bool _done;
};
