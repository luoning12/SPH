// MFC_OSG.cpp : implementation of the cOSG class
//
#include "stdafx.h"
#include "MFC_OSG.h"


cOSG::cOSG(HWND hWnd)
	:m_hWnd(hWnd)
	//...........................
	,m_SphereNum(0)
	,m_FlagSetSphereNum(FALSE)
	,m_FlagSetSphereProperty(FALSE)
	,m_FlagSetStickMove(FALSE)
	,m_FlagSetTextStep(FALSE)
{
}

cOSG::~cOSG()
{
    mViewer->setDone(true);
    Sleep(1000);
    mViewer->stopThreading();

    delete mViewer;
}

void cOSG::InitOSG()
{
    // Init different parts of OSG
    InitManipulators();
    InitSceneGraph();
    InitCameraConfig();
}

bool cOSG::SetSphereNum(int num)
{
	m_SphereNum = num;
	m_FlagSetSphereNum = TRUE;
	return TRUE;
}

bool cOSG::SetSphereProperty(int index, double position[3], double color[3])
{
	if (index >= m_SphereVector.size())
	{
		return FALSE;
	}
	
	osg::Vec3 vec_position = osg::Vec3(position[0], position[1], position[2]);
	osg::Vec4 vec_color = osg::Vec4(color[0]/255.0, color[1]/255.0, color[2]/255.0, 1.0);
	m_SphereVector.at(index).SetProperty(vec_position, vec_color);

	m_FlagSetSphereProperty = TRUE;
	return TRUE;
}

bool cOSG::SetStickMove(double position[6])
{
	m_StickMoveMatrix =
		osg::Matrix::rotate(osg::DegreesToRadians(position[3]), 1, 0, 0)				
		*osg::Matrix::rotate(osg::DegreesToRadians(position[4]), 0, 1, 0)
		*osg::Matrix::rotate(osg::DegreesToRadians(position[5]), 0, 0, 1)
		*osg::Matrix::translate(position[0], position[1], position[2]);
	
	m_FlagSetStickMove = TRUE;
	return TRUE;
}

bool cOSG::SetTextStep(int currentstep/*, int totalstep*/)
{
	std::stringstream stream;

	stream<<currentstep;
	std::string currentstep_str = stream.str();
	stream.str("");  //清空流
	//stream<<totalstep;
	//std::string totalstep_str = stream.str();
	//stream.str("");  //清空流

	m_StepText = std::string("Current Step: ") + currentstep_str/* + std::string("\nTotal Step: ") + totalstep_str*/;

	m_FlagSetTextStep = TRUE;
	return TRUE;
}


void cOSG::InitManipulators(void)
{
    // Create a trackball manipulator
    trackball = new osgGA::TrackballManipulator();

    // Create a Manipulator Switcher
    keyswitchManipulator = new osgGA::KeySwitchMatrixManipulator;

    // Add our trackball manipulator to the switcher
    keyswitchManipulator->addMatrixManipulator( '1', "Trackball", trackball.get());

    // Init the switcher to the first manipulator (in this case the only manipulator)
    keyswitchManipulator->selectMatrixManipulator(0);  // Zero based index Value

	//操纵器初始位置
	osg::Vec3 eye(-800, -600, 200);
	osg::Vec3 center(0, 0, 100);
	osg::Vec3 up(0, 0, 1);
	trackball->setHomePosition(eye,center,up);
}


void cOSG::InitSceneGraph(void)
{
    m_Root  = new osg::Group;
	m_SphereGroup = new osg::Group;

	m_Grid = osgDB::readNodeFile("./model/grid.osg");	
	m_Grid->asTransform()->asMatrixTransform()->setMatrix(osg::Matrix::scale(0.5, 0.5, 0.5));

	m_StickMove = new osg::MatrixTransform;
	m_StickModel = osgDB::readNodeFile("./model/stick.osg");
	m_StickMove->addChild(m_StickModel);

	m_Root->addChild(m_SphereGroup);
	m_Root->addChild(m_Grid);
	m_Root->addChild(m_StickMove);

	//============创建HUD文字============

	//创建文字节点
	m_Text = new osgText::Text;
	//设置字体
	m_Text->setFont("C:\\WINDOWS\\Fonts\\msyh.ttf");	//微软雅黑
	//设置文字显示的位置
	osg::Vec3 position(0.0f,0.0f,0.0f);
	m_Text->setPosition(position);   
	m_Text->setColor(osg::Vec4( 1, 1, 1, 1));
	m_Text->setText(L"");
	m_Text->setFontResolution(60, 60);
	m_Text->setCharacterSize(20);

	//几何体节点
	osg::Geode* geode = new osg::Geode();
	geode->addDrawable(m_Text);
	//设置状态
	osg::StateSet* stateset = geode->getOrCreateStateSet(); 
	stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);		//关闭灯光
	stateset->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);		//关闭深度测试
	//打开GL_BLEND混合模式（以保证Alpha纹理正确）
	stateset->setMode(GL_BLEND, osg::StateAttribute::ON);

	//创建HUD相机
	m_HUDCamera = new osg::Camera;
	//设置透视矩阵
	//m_HUDCamera->setProjectionMatrix(osg::Matrix::ortho2D(0,2000,0,2000));	//正交投影   
	//设置绝对参考坐标系，确保视图矩阵不会被上级节点的变换矩阵影响
	m_HUDCamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	//视图矩阵为默认的
	m_HUDCamera->setViewMatrix(osg::Matrix::identity());
	//设置背景为透明，否则的话可以设置ClearColor 
	m_HUDCamera->setClearMask(GL_DEPTH_BUFFER_BIT);
	//不响应事件，始终得不到焦点
	m_HUDCamera->setAllowEventFocus(false);
	//设置渲染顺序，必须在最后渲染
	m_HUDCamera->setRenderOrder(osg::Camera::POST_RENDER); 

	m_HUDCamera->addChild(geode);//将要显示的Geode节点加入到相机
	m_Root->addChild(m_HUDCamera);

    //// Optimize the model
    //osgUtil::Optimizer optimizer;
    //optimizer.optimize(m_SphereGroup.get());
    //optimizer.reset();

}

void cOSG::InitCameraConfig(void)
{
    // Local Variable to hold window size data
    RECT rect;

    // Create the viewer for this window
    mViewer = new osgViewer::Viewer();

    // Add a Stats Handler to the viewer
    mViewer->addEventHandler(new osgViewer::StatsHandler);

    // Get the current window size
    ::GetWindowRect(m_hWnd, &rect);

    // Init the GraphicsContext Traits
    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;

    // Init the Windata Variable that holds the handle for the Window to display OSG in.
    osg::ref_ptr<osg::Referenced> windata = new osgViewer::GraphicsWindowWin32::WindowData(m_hWnd);

    // Setup the traits parameters
	traits->x = 0;
	traits->y = 0;
    traits->width = rect.right - rect.left;
    traits->height = rect.bottom - rect.top;
    traits->windowDecoration = false;
    traits->doubleBuffer = true;
    traits->sharedContext = 0;
    traits->setInheritedWindowPixelFormat = true;
    traits->inheritedWindowData = windata;

    // Create the Graphics Context
    osg::GraphicsContext* gc = osg::GraphicsContext::createGraphicsContext(traits.get());

	//修正OSG3.4.1版本的BUG：由于运行osg::GraphicsContext::createGraphicsContext()函数会改变traits的值，所以在这里重新赋值
	traits->x = 0;
	traits->y = 0;

    // Init Master Camera for this View
    osg::ref_ptr<osg::Camera> camera = mViewer->getCamera();

    // Assign Graphics Context to the Camera
    camera->setGraphicsContext(gc);

    // Set the viewport for the Camera
    camera->setViewport(new osg::Viewport(traits->x, traits->y, traits->width, traits->height));

    // Set projection matrix and camera attribtues
    camera->setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    camera->setClearColor(osg::Vec4f(0.3f, 0.3f, 0.3f, 1.0f));
    camera->setProjectionMatrixAsPerspective(
        30.0f, static_cast<double>(traits->width)/static_cast<double>(traits->height), 1.0, 1000.0);

	m_HUDCamera->setProjectionMatrixAsOrtho2D(traits->x, traits->width, traits->y, traits->height);

    // Add the Camera to the Viewer
    //mViewer->addSlave(camera.get());
    mViewer->setCamera(camera.get());

    // Add the Camera Manipulator to the Viewer
    mViewer->setCameraManipulator(keyswitchManipulator.get());

    // Set the Scene Data
    mViewer->setSceneData(m_Root.get());

    // Realize the Viewer
    mViewer->realize();

    // Correct aspect ratio
    /*double fovy,aspectRatio,z1,z2;
    mViewer->getCamera()->getProjectionMatrixAsPerspective(fovy,aspectRatio,z1,z2);
    aspectRatio=double(traits->width)/double(traits->height);
    mViewer->getCamera()->setProjectionMatrixAsPerspective(fovy,aspectRatio,z1,z2);*/
}

void cOSG::PreFrameUpdate()
{
	if (m_FlagSetSphereNum)
	{
		//清空
		int num = m_SphereGroup->getNumChildren();
		m_SphereGroup->removeChildren(0, num);
		m_SphereVector.clear();

		//创建
		for (int i=0; i<m_SphereNum; i++)
		{
			MySphere temp;
			m_SphereVector.push_back(temp);
			m_SphereGroup->addChild(temp.GetNode());
		}
		m_FlagSetSphereNum = FALSE;
	}
	
	if (m_FlagSetSphereProperty)
	{
		int num = m_SphereVector.size();
		for (int i=0; i<num; i++)
		{
			m_SphereVector.at(i).UpdateProperty();
		}
		m_FlagSetSphereProperty = FALSE;
	}

	if (m_FlagSetStickMove)
	{
		m_StickMove->setMatrix(m_StickMoveMatrix);
		m_FlagSetStickMove = FALSE;
	}

	if (m_FlagSetTextStep)
	{
		m_Text->setText(m_StepText);
		m_FlagSetTextStep = FALSE;
	}

	//调整HUD相机投影矩阵，保证窗口拉伸时文字不变形
	const osg::GraphicsContext::Traits* traits = mViewer->getCamera()->getGraphicsContext()->getTraits();
	m_HUDCamera->setProjectionMatrixAsOrtho2D(0, traits->width, 0, traits->height);
	m_Text->setPosition(osg::Vec3(10, traits->height-30, 0));

}

void cOSG::PostFrameUpdate()
{
}


osg::Geode* cOSG::CreateSphere()
{
	osg::ref_ptr<osg::Geode> unitSphere = new osg::Geode;
	osg::ref_ptr<osg::Sphere> sphere = new osg::Sphere(osg::Vec3(0,0,0), 1.0f);
	osg::ref_ptr<osg::ShapeDrawable> shapeDrawable = new osg::ShapeDrawable(sphere.get());
	unitSphere->addDrawable(shapeDrawable.get());

	return unitSphere.release();
}





CRenderingThread::CRenderingThread( cOSG* ptr )
:   OpenThreads::Thread(), _ptr(ptr), _done(false)
{
}

CRenderingThread::~CRenderingThread()
{
    _done = true;
    if (isRunning())
    {
        cancel();
        join();
    }
}

void CRenderingThread::run()
{
    if ( !_ptr )
    {
        _done = true;
        return;
    }

    osgViewer::Viewer* viewer = _ptr->getViewer();
    do
    {
        _ptr->PreFrameUpdate();
        viewer->frame();
        _ptr->PostFrameUpdate();
    } while ( !testCancel() && !viewer->done() && !_done );
}




