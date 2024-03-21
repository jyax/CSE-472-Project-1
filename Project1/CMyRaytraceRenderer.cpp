#include "pch.h"
#include "CMyRaytraceRenderer.h"

void CMyRaytraceRenderer::SetWindow(CWnd* p_window)
{
    m_window = p_window;
}

bool CMyRaytraceRenderer::RendererStart()
{
	m_intersection.Initialize();

	m_mstack.clear();


	// We have to do all of the matrix work ourselves.
	// Set up the matrix stack.
	CGrTransform t;
	t.SetLookAt(Eye().X(), Eye().Y(), Eye().Z(),
		Center().X(), Center().Y(), Center().Z(),
		Up().X(), Up().Y(), Up().Z());

	m_mstack.push_back(t);

	for (int i = 0; i < CGrRenderer::LightCnt(); i++) {
		CGrRenderer::Light light = GetLight(i);
		light.m_pos = t * light.m_pos;
		m_lights.push_back(light);
	}


	m_material = NULL;

	return true;
}

void CMyRaytraceRenderer::RendererMaterial(CGrMaterial* p_material)
{
	m_material = p_material;
}

void CMyRaytraceRenderer::RendererPushMatrix()
{
	m_mstack.push_back(m_mstack.back());
}

void CMyRaytraceRenderer::RendererPopMatrix()
{
	m_mstack.pop_back();
}

void CMyRaytraceRenderer::RendererRotate(double a, double x, double y, double z)
{
	CGrTransform r;
	r.SetRotate(a, CGrPoint(x, y, z));
	m_mstack.back() *= r;
}

void CMyRaytraceRenderer::RendererTranslate(double x, double y, double z)
{
	CGrTransform r;
	r.SetTranslate(x, y, z);
	m_mstack.back() *= r;
}

//
// Name : CMyRaytraceRenderer::RendererEndPolygon()
// Description : End definition of a polygon. The superclass has
// already collected the polygon information
//

void CMyRaytraceRenderer::RendererEndPolygon()
{
    const std::list<CGrPoint>& vertices = PolyVertices();
    const std::list<CGrPoint>& normals = PolyNormals();
    const std::list<CGrPoint>& tvertices = PolyTexVertices();

    // Allocate a new polygon in the ray intersection system
    m_intersection.PolygonBegin();
    m_intersection.Material(m_material);

    if (PolyTexture())
    {
        m_intersection.Texture(PolyTexture());
    }

    std::list<CGrPoint>::const_iterator normal = normals.begin();
    std::list<CGrPoint>::const_iterator tvertex = tvertices.begin();

    for (std::list<CGrPoint>::const_iterator i = vertices.begin(); i != vertices.end(); i++)
    {
        if (normal != normals.end())
        {
            m_intersection.Normal(m_mstack.back() * *normal);
            normal++;
        }

        if (tvertex != tvertices.end())
        {
            m_intersection.TexVertex(*tvertex);
            tvertex++;
        }

        m_intersection.Vertex(m_mstack.back() * *i);
    }

    m_intersection.PolygonEnd();
}

bool CMyRaytraceRenderer::RendererEnd()
{
	m_intersection.LoadingComplete();

	double ymin = -tan(ProjectionAngle() / 2 * GR_DTOR);
	double yhit = -ymin * 2;

	double xmin = ymin * ProjectionAspect();
	double xwid = -xmin * 2;

	for (int r = 0; r < m_rayimageheight; r++)
	{
		for (int c = 0; c < m_rayimagewidth; c++)
		{
			double colorTotal[3] = { 0, 0, 0 };

			double x = xmin + (c + 0.5) / m_rayimagewidth * xwid;
			double y = ymin + (r + 0.5) / m_rayimageheight * yhit;


			// Construct a Ray
			CRay ray(CGrPoint(0, 0, 0), Normalize3(CGrPoint(x, y, -1, 0)));

			double t;                                   // Will be distance to intersection
			CGrPoint intersect;                         // Will by x,y,z location of intersection
			const CRayIntersection::Object* nearest;    // Pointer to intersecting object
			if (m_intersection.Intersect(ray, 1e20, NULL, nearest, t, intersect))
			{
				// We hit something...
				// Determine information about the intersection
				CGrPoint N;
				CGrMaterial* material;
				CGrTexture* texture;
				CGrPoint texcoord;

				m_intersection.IntersectInfo(ray, nearest, t,
					N, material, texture, texcoord);

				if (material != NULL)
				{
					double raytexture[3] = { 0,0,0 };
					double raycolor[3] = { 0,0,0 };

					RayColor(ray, m_recurse, raycolor, t);
					if (m_textureOn)
					{
						RayTexture(texture, texcoord, raytexture);
					}

					for (int c = 0; c < 3; c++) {
						colorTotal[c] += raycolor[c] + raytexture[c];
						if (colorTotal[c] > 1) colorTotal[c] = 1;
					}


					m_rayimage[r][c * 3] = BYTE(colorTotal[0] * 255);
					m_rayimage[r][c * 3 + 1] = BYTE(colorTotal[1] * 255);
					m_rayimage[r][c * 3 + 2] = BYTE(colorTotal[2] * 255);
				}

			}
			else
			{
				// We hit nothing...
				m_rayimage[r][c * 3] = 0;
				m_rayimage[r][c * 3 + 1] = 0;
				m_rayimage[r][c * 3 + 2] = 0;
			}
		}
		if ((r % 50) == 0)
		{
			m_window->Invalidate();
			MSG msg;
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
				DispatchMessage(&msg);
		}
	}


	return true;
}


double CMyRaytraceRenderer::RayColor(CRay ray, int recurse, double* raycolor, double t) {

	CGrPoint intersect;                         // Will by x,y,z location of intersection
	const CRayIntersection::Object* nearest;    // Pointer to intersecting object

	if (m_intersection.Intersect(ray, 1e20, nearest, nearest, t, intersect))
	{
		// We hit something...
		// Determine information about the intersection
		CGrPoint N;
		CGrMaterial* material;
		CGrTexture* texture;
		CGrPoint texcoord;

		m_intersection.IntersectInfo(ray, nearest, t, N, material, texture, texcoord);

		if (material != NULL)
		{
			//
			// AMBIENT LIGHT
			//
			if (m_ambientOn) {
				for (int c = 0; c < 3; c++) {
					raycolor[c] = m_lights[0].m_ambient[c] * material->Ambient(c);
					if (raycolor[c] > 1) raycolor[c] = 1;
				}
			}

			for (int i = 0; i < CGrRenderer::LightCnt(); i++) {
				CGrRenderer::Light light = m_lights[i];
				double cameraPosition[3] = { Eye()[0],Eye()[1],Eye()[2] };
				double cameraDirection[3] = { intersect[0] - cameraPosition[0], intersect[1] - cameraPosition[1], intersect[2] - cameraPosition[2] };

				//
				// SPECULAR REFLECTIONS
				//
				if (recurse > 1 && (material->Specular(0) > 0 || material->Specular(1) > 0 || material->Specular(2) > 0)) {
					CGrPoint V = cameraDirection;
					CGrPoint R = N * 2 * dotProduct(N, V) - V;
					CRay Ray(intersect, R);
					double d_color[3] = { 0,0,0 };
					double nt = t;
					RayColor(ray, recurse - 1, d_color, nt);
					for (int c = 0; c < 3; c++) {
						raycolor[c] += material->Specular(c) * d_color[c] * pow(t, nt);
						if (raycolor[c] > 1) raycolor[c] = 1;
					}

				}

				//
				// lightDirection
				//
				double lightDirection[4] = { 0.,0.,0.,0. };
				if (light.m_pos[3] == 0)
					for (int c = 0; c < 4; c++) {
						lightDirection[c] = light.m_pos[c];
					}
				else {
					for (int c = 0; c < 4; c++) {
						lightDirection[c] = light.m_pos[c] - intersect[c];
					}
				}
				for (int c = 0; c < 4; c++) {
					lightDirection[c] = Normalize(lightDirection)[c];
				}

				//
				// SHADOW RAY
				//
				if (m_shadow) {
					CRay shadowRay(intersect, lightDirection);
					double shadowT;
					CGrPoint shadowIntersect;
					const CRayIntersection::Object* shadowObj;
					if (m_intersection.Intersect(shadowRay, 1e20, nearest, shadowObj, shadowT, shadowIntersect)) {
						continue;
					}
				}
				double surfaceNormal[4] = { N[0], N[1], N[2], N[3] };
				if (dotProduct(surfaceNormal, lightDirection) < 0) {
					continue;
				}

				//
				// DIFFUSE LIGHT
				//
				if (m_diffuseOn) {
					for (int c = 0; c < 3; c++) {
						raycolor[c] += light.m_diffuse[c] * material->Diffuse(c) * dotProduct(surfaceNormal, lightDirection);
						if (raycolor[c] > 1) raycolor[c] = 1;
					}

				}

				double halfVector[3] = { lightDirection[0] + cameraDirection[0], lightDirection[1] + cameraDirection[1], lightDirection[2] + cameraDirection[2] };
				for (int c = 0; c < 3; c++) {
					halfVector[c] = Normalize(halfVector)[c];
				}
				
				//
				// SPECULAR LIGHT
				//
				if (m_specOn) {
					for (int c = 0; c < 3; c++) {
						raycolor[c] += light.m_specular[c] * material->Specular(c) * pow(dotProduct(surfaceNormal, halfVector), material->Shininess());
						if (raycolor[c] > 1) raycolor[c] = 1;
					}

				}
			}
		}
	}
	return t;

}

void CMyRaytraceRenderer::RayTexture(CGrTexture* texture, CGrPoint texcoord, double* raytexture)
{
	if (texture != NULL && !texture->Empty())
	{
		int U = static_cast<int>(texcoord[0] * texture->Width());
		int V = static_cast<int>(texcoord[1] * texture->Height());
		int x0 = U / 1;
		int x1 = x0 + 1;
		int y0 = V / 1;
		int y1 = y0 + 1;

		if (x1 >= texture->Width()) { 
			x1 = texture->Width() - 1; 
		} if (y1 >= texture->Height()) {
			y1 = texture->Height() - 1;
		}

		double x = texcoord[0] * texture->Width(); 
		double y = texcoord[1] * texture->Height(); 
		double u = std::modf(x, &x);
		double v = std::modf(y, &y);
		double w[4] = { (1 - u) * (1 - v), u * (1 - v), (1 - u) * v, u * v };

		double* c1 = getPiexelColor(U, V, texture);
		double* c2 = getPiexelColor(x0, y1, texture);
		double* c3 = getPiexelColor(x1, y0, texture);
		double* c4 = getPiexelColor(x1, y1, texture);
		double lightambien[3] = { 0.3,0.3,0.3 };

		for (int k = 0; k < 3; k++)
		{
			raytexture[k] = (w[0] * c1[k] + w[1] * c2[k] + w[2] * c3[k] + w[3] * c4[k]) * lightambien[k];
			if (raytexture[k] > 1) { 
				raytexture[k] = 1; 
			}
		}

	}
}

double CMyRaytraceRenderer::Length(double* vec)
{
	int len = sizeof(vec);
	double sum = 0.0;
	for (int i = 0; i < len; i++) {
		sum += vec[i] * vec[i];
	}
	return sqrt(sum);
}

double* CMyRaytraceRenderer::getPiexelColor(int u, int v, CGrTexture* texture)
{
	int index = v;
	int r = int(texture->Row(index)[u * 3]);
	int g = int(texture->Row(index)[u * 3 + 1]);
	int b = int(texture->Row(index)[u * 3 + 2]);
	double rgb[3] = { r / 255.0,g / 255.0,b / 255.0 };
	return rgb;
}

double CMyRaytraceRenderer::dotProduct(double* p1, double* p2)
{
	int len = sizeof(p1);
	double ret = 0.0;
	for (int i = 0; i < len; i++) {
		ret += p1[i] * p2[i];
	}
	return ret;
}

double* CMyRaytraceRenderer::Normalize(double* vec)
{
	const int len = sizeof(vec);
	double length = Length(vec);

	double ret[len] = { 0.0 };
	for (int i = 0; i < len; i++) {
		ret[i] = vec[i] / length;
	}
	return ret;
}

