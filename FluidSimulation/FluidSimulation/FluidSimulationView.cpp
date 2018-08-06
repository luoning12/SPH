
// FluidSimulationView.cpp : CFluidSimulationView 类的实现
//

#include "stdafx.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "FluidSimulation.h"
#endif

#include "FluidSimulationDoc.h"
#include "FluidSimulationView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CFluidSimulationView

IMPLEMENT_DYNCREATE(CFluidSimulationView, CView)

BEGIN_MESSAGE_MAP(CFluidSimulationView, CView)
	// 标准打印命令
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CFluidSimulationView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_WM_CREATE()
END_MESSAGE_MAP()

// CFluidSimulationView 构造/析构

CFluidSimulationView::CFluidSimulationView()
{
	// TODO: 在此处添加构造代码

}

CFluidSimulationView::~CFluidSimulationView()
{
	delete mThreadHandle;
	if (mOSG != 0)
	{
		delete mOSG;
	}
	if (sph)
	{
		delete sph;
	}
}

BOOL CFluidSimulationView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	return CView::PreCreateWindow(cs);
}

// CFluidSimulationView 绘制

void CFluidSimulationView::OnDraw(CDC* /*pDC*/)
{
	CFluidSimulationDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: 在此处为本机数据添加绘制代码
}


// CFluidSimulationView 打印


void CFluidSimulationView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CFluidSimulationView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 默认准备
	return DoPreparePrinting(pInfo);
}

void CFluidSimulationView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加额外的打印前进行的初始化过程
}

void CFluidSimulationView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加打印后进行的清理过程
}

void CFluidSimulationView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CFluidSimulationView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CFluidSimulationView 诊断

#ifdef _DEBUG
void CFluidSimulationView::AssertValid() const
{
	CView::AssertValid();
}

void CFluidSimulationView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CFluidSimulationDoc* CFluidSimulationView::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CFluidSimulationDoc)));
	return (CFluidSimulationDoc*)m_pDocument;
}
#endif //_DEBUG


// CFluidSimulationView 消息处理程序


int CFluidSimulationView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  在此添加您专用的创建代码
	mOSG = new cOSG(m_hWnd);

	return 0;
}


void CFluidSimulationView::OnInitialUpdate()
{
	CView::OnInitialUpdate();
	mOSG->InitOSG();

	// Start the thread to do OSG Rendering
	
	mThreadHandle = new CRenderingThread(mOSG);
	mThreadHandle->start();
	Sleep(500);

	sph = new CSPH();
	sph->Init(4000);
	sph->run(mOSG);



	// TODO: 在此添加专用代码和/或调用基类
}


	//test
	/*int number = 1000;
	mOSG->SetSphereNum(number);
	Sleep(500);


	double pos[3] = {0, 0, 0};
	double color[3] = {0, 0, 0};
	int index = 0;

	for (double i=0; i<10; i++)
	{
		pos[0] += 100.0/9; 
		color[0] += 255.0/9;

		for (int j=0; j<10; j++)
		{
			pos[1] += 100.0/9;
			color[1] += 255.0/9;

			for(int k=0; k<10; k++)
			{
				pos[2] += 100.0/9;
				color[2] += 255.0/9;

				mOSG->SetSphereProperty(index, pos, color);
				index++;
			}
			pos[2] = 0;
			color[2] = 0;
		}
		pos[1] = 0;
		color[1] = 0;
	}*/