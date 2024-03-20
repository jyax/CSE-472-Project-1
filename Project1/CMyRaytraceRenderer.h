#pragma once
#include "graphics/GrRenderer.h"
#include "graphics/RayIntersection.h"
#include "graphics/GrCamera.h"
#include "graphics/GrTexture.h".


class CMyRaytraceRenderer :
	public CGrRenderer
{
public:
    CMyRaytraceRenderer() { m_window = NULL; }
    int     m_rayimagewidth;
    int     m_rayimageheight;
    BYTE** m_rayimage;
    void SetImage(BYTE** image, int w, int h) { m_rayimage = image; m_rayimagewidth = w;  m_rayimageheight = h; }

    CWnd* m_window;

    CRayIntersection m_intersection;

    std::list<CGrTransform> m_mstack;
    CGrMaterial* m_material;

    CGrCamera m_camera;
    void SetCamera(CGrCamera camera) { m_camera = camera; }

    void SetWindow(CWnd* p_window);
    bool RendererStart();
    bool RendererEnd();
    void RendererMaterial(CGrMaterial* p_material);

    double dotProduct(double*, double*);
    double RayColor(CRay, int, double*, double);
    void RayTexture(CGrTexture*, CGrPoint, double*);
    double* Normalize(double*);
    double* getPiexelColor(int, int, CGrTexture*);
    double Length(double*);
    bool m_diffuseOn = TRUE;
    bool m_specOn = TRUE;
    bool m_ambientOn = TRUE;
    bool m_textureOn = TRUE;
    bool m_shadow = TRUE;
    int m_recurse = 2;
    std::vector<Light> m_lights;

    virtual void RendererPushMatrix();
    virtual void RendererPopMatrix();
    virtual void RendererRotate(double a, double x, double y, double z);
    virtual void RendererTranslate(double x, double y, double z);
    void RendererEndPolygon();

};

