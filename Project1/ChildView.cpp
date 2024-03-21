
// ChildView.cpp : implementation of the CChildView class
//

#include "pch.h"
#include "framework.h"
#include "Project1.h"
#include "ChildView.h"
#include "graphics/OpenGLRenderer.h"
#include "CMyRaytraceRenderer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CChildView

CChildView::CChildView()
{
	m_camera.Set(20., 10., 50., 0., 0., 0., 0., 1., 0.);

	m_raytrace = false;
	m_rayimage = NULL;

	CGrPtr<CGrComposite> scene = new CGrComposite;
	m_scene = scene;

	// A red box
//	CGrPtr<CGrMaterial> redpaint = new CGrMaterial;
//	redpaint->AmbientAndDiffuse(0.8f, 0.0f, 0.0f);
//	scene->Child(redpaint);

//	CGrPtr<CGrComposite> redbox = new CGrComposite;
//	redpaint->Child(redbox);
//	redbox->Box(1, 1, 1, 5, 5, 5);

	// A white box
	CGrPtr<CGrMaterial> whitepaint = new CGrMaterial;
	whitepaint->AmbientAndDiffuse(0.8f, 0.8f, 0.8f);
	whitepaint->Specular(0.5f, 0.5f, 0.5f);
	scene->Child(whitepaint);

	CGrPtr<CGrComposite> whitebox = new CGrComposite;
	whitepaint->Child(whitebox);
	whitebox->Box(-10, 2, -10, 5, 5, 5);

	CGrPtr<CGrTexture> m_texture = new CGrTexture();
	m_texture->LoadFile(L"textures/plank01.bmp");

	CGrPtr<CGrTexture> m_texture2 = new CGrTexture();
	m_texture2->LoadFile(L"textures/floor.bmp");

	// A new box
	CGrPtr<CGrMaterial> item1paint = new CGrMaterial;
	item1paint->AmbientAndDiffuse(0.4f, 0.4f, 0.4f);
	item1paint->Specular(0.5f, 0.5f, 0.5f);
	scene->Child(item1paint);

	CGrPtr<CGrComposite> item1 = new CGrComposite;
	item1paint->Child(item1);
	item1->SlantBox(-10, 0, 2, 2, 2, 2, 1);

	// A plane
	CGrPtr<CGrMaterial> item3paint = new CGrMaterial;
	item3paint->AmbientAndDiffuse(0.4f, 0.4f, 0.4f);
	scene->Child(item3paint);

	CGrPtr<CGrComposite> item3 = new CGrComposite;
	item3paint->Child(item3);
	item3->Box(-15, -3, -20, 30, 3, 40, m_texture2);


	// Adding a Cylinder
	CGrPtr<CGrMaterial> Cylinderpaint = new CGrMaterial;
	Cylinderpaint->AmbientAndDiffuse(0.5, 0.5f, 0.5f);
	//Cylinderpaint->Specular(0.5f, 0.5f, 0.5f);
	scene->Child(Cylinderpaint);

	CGrPtr<CGrComposite> cylinder = new CGrComposite;
	Cylinderpaint->Child(cylinder);
	cylinder->Cylinder(0, 5, 0, 5, 15, 1000, m_texture);

	CGrPtr<CGrMaterial> boxMaterial = new CGrMaterial;
	boxMaterial->AmbientAndDiffuse(0.8f, 0.8f, 0.8f);
	boxMaterial->Specular(0.5f, 0.5f, 0.5f);
	scene->Child(boxMaterial);

	CGrPtr<CGrComposite> box = new CGrComposite;
	boxMaterial->Child(box);
	box->Box(-10, 2, -10, 5, 5, 5);
}

CChildView::~CChildView()
{
}


BEGIN_MESSAGE_MAP(CChildView, COpenGLWnd)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_COMMAND(ID_RENDER_RAYTRACE, &CChildView::OnRenderRaytrace)
	ON_UPDATE_COMMAND_UI(ID_RENDER_RAYTRACE, &CChildView::OnUpdateRenderRaytrace)
END_MESSAGE_MAP()



// CChildView message handlers

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!COpenGLWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(nullptr, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), nullptr);

	return TRUE;
}

void CChildView::OnGLDraw(CDC* pDC)
{
	if (m_raytrace)
	{
		// Clear the color buffer
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Set up for parallel projection
		int width, height;
		GetSize(width, height);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, width, 0, height, -1, 1);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		// If we got it, draw it
		if (m_rayimage)
		{
			glRasterPos3i(0, 0, 0);
			glDrawPixels(m_rayimagewidth, m_rayimageheight,
				GL_RGB, GL_UNSIGNED_BYTE, m_rayimage[0]);
		}

		glFlush();
	}
	else
	{
		//
		// Instantiate a renderer
		//

		COpenGLRenderer renderer;

		// Configure the renderer
		ConfigureRenderer(&renderer);

		//
		// Render the scene
		//

		renderer.Render(m_scene);
	}
}

void CChildView::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_camera.MouseDown(point.x, point.y);

	COpenGLWnd::OnLButtonDown(nFlags, point);
}


void CChildView::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_camera.MouseMove(point.x, point.y, nFlags))
		Invalidate();

	COpenGLWnd::OnMouseMove(nFlags, point);
}


void CChildView::OnRButtonDown(UINT nFlags, CPoint point)
{
	m_camera.MouseDown(point.x, point.y, 2);

	COpenGLWnd::OnRButtonDown(nFlags, point);
}

//
// Name :         CChildView::ConfigureRenderer()
// Description :  Configures our renderer so it is able to render the scene.
//                Indicates how we'll do our projection, where the camera is,
//                and where any lights are located.
//

void CChildView::ConfigureRenderer(CGrRenderer* p_renderer)
{
	// Determine the screen size so we can determine the aspect ratio
	int width, height;
	GetSize(width, height);
	double aspectratio = double(width) / double(height);

	//
	// Set up the camera in the renderer
	//

	p_renderer->Perspective(m_camera.FieldOfView(),
		aspectratio, // The aspect ratio.
		20., // Near clipping
		1000.); // Far clipping

	// m_camera.FieldOfView is the vertical field of view in degrees.

	//
	// Set the camera location
	//

	p_renderer->LookAt(m_camera.Eye()[0], m_camera.Eye()[1], m_camera.Eye()[2],
		m_camera.Center()[0], m_camera.Center()[1], m_camera.Center()[2],
		m_camera.Up()[0], m_camera.Up()[1], m_camera.Up()[2]);

	//
	// Set the light locations and colors
	//

	float dimd = 0.5f;
	GLfloat dim[] = { dimd, dimd, dimd, 1.0f };
	GLfloat brightwhite[] = { 1.f, 1.f, 1.f, 1.0f };
	GLfloat darkwhite[] = { 0.2, 0.2, 0.2, 1.0f };

	p_renderer->AddLight(CGrPoint(15, 0.5, 0, 0),
		NULL, brightwhite, brightwhite);

	p_renderer->AddLight(CGrPoint(0, 10, 0, 0),
		NULL, darkwhite, brightwhite);

	p_renderer->AddLight(CGrPoint(0, 5.2, 20, 0),
		NULL, darkwhite, brightwhite);
}


void CChildView::OnRenderRaytrace()
{
	m_raytrace = !m_raytrace;
	Invalidate();
	if (!m_raytrace)
		return;

	GetSize(m_rayimagewidth, m_rayimageheight);

	m_rayimage = new BYTE * [m_rayimageheight];

	int rowwid = m_rayimagewidth * 3;
	while (rowwid % 4)
		rowwid++;

	m_rayimage[0] = new BYTE[m_rayimageheight * rowwid];
	for (int i = 1; i < m_rayimageheight; i++)
	{
		m_rayimage[i] = m_rayimage[0] + i * rowwid;
	}

	for (int i = 0; i < m_rayimageheight; i++)
	{
		// Fill the image with blue
		for (int j = 0; j < m_rayimagewidth; j++)
		{
			m_rayimage[i][j * 3] = 0;               // red
			m_rayimage[i][j * 3 + 1] = 0;           // green
			m_rayimage[i][j * 3 + 2] = BYTE(255);   // blue
		}
	}
	
	// Instantiate a raytrace object
	CMyRaytraceRenderer raytrace;

	// Generic configurations for all renderers
	ConfigureRenderer(&raytrace);

	//
	// Render the Scene
	//
	raytrace.SetImage(m_rayimage, m_rayimagewidth, m_rayimageheight);
	raytrace.SetWindow(this);
	raytrace.Render(m_scene);
	Invalidate();
}


void CChildView::OnUpdateRenderRaytrace(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_raytrace);
}
