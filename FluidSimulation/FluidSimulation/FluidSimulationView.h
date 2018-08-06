
// FluidSimulationView.h : CFluidSimulationView 类的接口
//

#pragma once
#include "MFC_OSG.h"
#include "SPH.h"


class CFluidSimulationView : public CView
{
protected: // 仅从序列化创建
	CFluidSimulationView();
	DECLARE_DYNCREATE(CFluidSimulationView)

// 特性
public:
	CFluidSimulationDoc* GetDocument() const;

	cOSG *mOSG;
	CRenderingThread* mThreadHandle;

	CSPH *sph;



// 操作
public:

// 重写
public:
	virtual void OnDraw(CDC* pDC);  // 重写以绘制该视图
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// 实现
public:
	virtual ~CFluidSimulationView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 生成的消息映射函数
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual void OnInitialUpdate();
};

#ifndef _DEBUG  // FluidSimulationView.cpp 中的调试版本
inline CFluidSimulationDoc* CFluidSimulationView::GetDocument() const
   { return reinterpret_cast<CFluidSimulationDoc*>(m_pDocument); }
#endif

