#include "stdafx.h"

#include "core/image.h"
#include "rt/basic_definitions.h"
#include "rt/geometry_group.h"

#include "impl/lwobject.h"
#include "impl/phong_shaders.h"
#include "impl/basic_primitives.h"
#include "impl/perspective_camera.h"
#include "impl/integrator.h"
#include "rt/renderer.h"
#include "impl/samplers.h"

class BumpMirrorPhongShader : public DefaultPhongShader
{
protected:
	float2 m_texCoord; 
	Point m_position;
	Vector m_tang, m_biNorm;
public:
	float reflCoef;

	virtual void setPosition(const Point& _point) { m_position = _point; }

	//Set the tangent to (0, 1, 0)
	virtual void setNormal(const Vector& _normal) 
	{
		DefaultPhongShader::setNormal(_normal);
		m_tang = Vector(0, -1, 0);
		m_biNorm = ~_normal % m_tang;
	}

	virtual Vector getNormal() const 
	{ 
		Vector ret = m_normal;

		float d;
		float2 t1 = float2(modf(m_texCoord.x, &d), modf(m_texCoord.y, &d)) - float2(0.5f, 0.5f);

		float dist = t1.x * t1.x  + t1.y * t1.y;
		const float R = 0.25;
		const float DIV = 10.f;
		if(dist < R * R)
		{
			Vector newNorm(t1.x / DIV, t1.y / DIV, sqrtf(R * R - dist / (DIV * DIV)));
			ret = newNorm.x * m_tang + newNorm.y * m_biNorm + newNorm.z * m_normal;
			ret = ~ret;
		}

		return ret;
	}

	virtual float4 getIndirectRadiance(const Vector &_out, Integrator *_integrator) const
	{ 
		Ray r;
		r.o = m_position;

		Vector n = getNormal();
		Vector v = n * fabs(n * _out);
		r.d = _out + 2 * (v - _out);

		return _integrator->getRadiance(r) * float4::rep(reflCoef);
	}

	virtual void setTextureCoord(const float2& _texCoord) { m_texCoord = _texCoord;}

	_IMPLEMENT_CLONE(BumpMirrorPhongShader);

};


void assigment4_1_and_2()
{
	Image img(800, 600);
	img.addRef();

	//Set up the scene
	GeometryGroup scene;
	LWObject cow;
	cow.read("models/cow.obj", true);
	cow.addReferencesToScene(scene.primitives);
	scene.rebuildIndex();

	BumpMirrorPhongShader sh4;
	sh4.diffuseCoef = float4(0.2f, 0.2f, 0, 0);
	sh4.ambientCoef = sh4.diffuseCoef;
	sh4.specularCoef = float4::rep(0.8f);
	sh4.specularExponent = 10000.f;
	sh4.reflCoef = 0.4f;
	sh4.addRef();
	cow.materials[cow.materialMap["Floor"]].shader = &sh4;
	
 	//Enable bi-linear filtering on the walls
	((TexturedPhongShader*)cow.materials[cow.materialMap["Stones"]].shader.data())->diffTexture->filterMode = Texture::TFM_Bilinear;
 	((TexturedPhongShader*)cow.materials[cow.materialMap["Stones"]].shader.data())->amibientTexture->filterMode = Texture::TFM_Bilinear;


	//Set up the cameras
	PerspectiveCamera cam1(Point(-9.398149f, -6.266083f, 5.348377f), Point(-6.324413f, -2.961229f, 4.203216f), Vector(0, 0, 1), 30,
		std::make_pair(img.width(), img.height()));

	PerspectiveCamera cam2(Point(2.699700f, 6.437226f, 0.878297f), Point(4.337114f, 8.457443f,- 0.019007f), Vector(0, 0, 1), 30,
		std::make_pair(img.width(), img.height()));
	
	cam1.addRef();
	cam2.addRef();

	//Set up the integrator
	IntegratorImpl integrator;
	integrator.addRef();
	integrator.scene = &scene;
	PointLightSource pls;

	pls.falloff = float4(0, 0, 1, 0);

	pls.intensity  = float4::rep(0.9f);
	pls.position = Point(-2.473637f, 3.119330f, 9.571486f);
	integrator.lightSources.push_back(pls);

	integrator.ambientLight = float4::rep(0.1f);

	DefaultSampler samp;
	samp.addRef();

	//Render
	Renderer r;
	r.integrator = &integrator;
	r.target = &img;
	r.sampler = &samp;

	r.camera = &cam1;
	r.render();
	img.writePNG("result_cam1.png");

	//For seeing the difference in texture filtering
	r.camera = &cam2;
	r.render();
	img.writePNG("result_cam2.png");
}

