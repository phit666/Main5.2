///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ZzzOpenglUtil.h"
#include "ZzzBMD.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzLodTerrain.h"
#include "ZzzTexture.h"
#include "ZzzAi.h"
#include "ZzzEffect.h"
#include "DSPlaySound.h"
#include "WSClient.h"
#include "mu_gles2_matrix.h"

#define MAX_BLURS      100
#define MAX_BLUR_TAILS 30
#define MAX_BLUR_LIFETIME 30

#define MAX_OBJECTBLURS		1000
#define MAX_OBJECTBLUR_TAILS 600
#define MAX_OBJECTBLUR_LIFETIME 30
//#define MAX_OBJECTBLUR_LIFETIME 11		// FIX

typedef struct
{
	bool      Live;
	int       Type;
	int       LifeTime;
	CHARACTER *Owner;
	int       Number;
	vec3_t    Light;
	vec3_t    p1[MAX_BLUR_TAILS];
	vec3_t    p2[MAX_BLUR_TAILS];
	int       SubType;
} BLUR;

BLUR Blur[MAX_BLURS];

void AddBlur(BLUR *b,vec3_t p1,vec3_t p2,vec3_t Light,int Type)
{
	b->Type = Type;
	VectorCopy(Light,b->Light);
	for(int i=b->Number-1;i>=0;i--)
	{
		VectorCopy(b->p1[i],b->p1[i+1]);
		VectorCopy(b->p2[i],b->p2[i+1]);
	}
	VectorCopy(p1,b->p1[0]);
	VectorCopy(p2,b->p2[0]);
	b->Number++;
	if(b->Number >= MAX_BLUR_TAILS-1)
    {
		b->Number = MAX_BLUR_TAILS-1;
    }
}

void CreateBlur(CHARACTER *Owner,vec3_t p1,vec3_t p2,vec3_t Light,int Type,bool Short,int SubType)
{
	for(int i=0;i<MAX_BLURS;i++)
	{
		BLUR *b = &Blur[i];
		if(b->Live && b->Owner==Owner)
		{
			if(SubType > 0 && b->SubType != SubType)
				continue;
			AddBlur(b,p1,p2,Light,Type);
            return;
		}
	}

	for(int i=0;i<MAX_BLURS;i++)
	{
		BLUR *b = &Blur[i];
		if(!b->Live)
		{
			b->Live = true;
			b->Owner = Owner;
            b->Number= 0;
			b->LifeTime = Short? 15 : MAX_BLUR_LIFETIME;
			b->SubType = SubType;
			AddBlur(b,p1,p2,Light,Type);
            return;
		}
	}
}

void MoveBlurs()
{
	for(int i=0;i<MAX_BLURS;i++)
	{
		BLUR *b = &Blur[i];
		if(b->Live)
		{
			b->LifeTime--;
			b->Number--;

	        for(int i=b->Number-1;i>=0;i--)
	        {
		        VectorCopy(b->p1[i],b->p1[i+1]);
		        VectorCopy(b->p2[i],b->p2[i+1]);
	        }
			if(b->LifeTime <= 0)
			{
				b->Number--;
				if(b->Number <= 0)
				{
        			b->Live = false;
				}
			}
		}
	}
	MoveObjectBlurs();
}

void RenderBlurs()
{
    int Type;
    //DisableCullFace();
	for(int i=0;i<MAX_BLURS;i++)
	{
		BLUR *b = &Blur[i];
		if(b->Live)
		{
            Type = b->Type;

			int nTexture = BITMAP_BLUR+Type;
			if(Type == 3)
			{
				nTexture = BITMAP_BLUR2;
			}
			else if(Type == 4)
			{
				nTexture = BITMAP_BLUR;
			}
			else if(Type == 5)
			{
				nTexture = BITMAP_BLUR+3;
			}
			if(b->Owner->Level == 0 && (Type<=3 || ( Type >= 5 && Type <= 10) ))
			{
              	EnableAlphaBlend();
			}
			else
			{
              	EnableAlphaBlendMinus();
			}

            if( Type>2 ) Type = Type - 3;
			if(b->Number >= 2)
			{
				BindTexture(nTexture);

				// 1. Prepare a buffer for all vertices
				// Each quad has 4 vertices, and we have (b->Number - 1) quads
				std::vector<SpriteVertexFull> vao;
				vao.reserve((b->Number - 1) * 4);

				for (int j = 0; j < b->Number - 1; j++)
				{
					float light1, light2;
					float texU1 = (float)j / (float)b->Number;
					float texU2 = (float)(j + 1) / (float)b->Number;

					if (b->Owner->Level == 0) {
						light1 = (float)(b->Number - j) / (float)b->Number;
						light2 = (float)(b->Number - (j + 1)) / (float)b->Number;
					}
					else {
						light1 = 1.0f;
						light2 = 1.0f;
					}

					// Vertex 1 (p1[j])
					vao.push_back({ b->p1[j][0], b->p1[j][1], b->p1[j][2], texU1, 1.0f, b->Light[0] * light1, b->Light[1] * light1, b->Light[2] * light1, 1.0f });
					// Vertex 2 (p2[j])
					vao.push_back({ b->p2[j][0], b->p2[j][1], b->p2[j][2], texU1, 0.0f, b->Light[0] * light1, b->Light[1] * light1, b->Light[2] * light1, 1.0f });
					// Vertex 3 (p2[j+1])
					vao.push_back({ b->p2[j + 1][0], b->p2[j + 1][1], b->p2[j + 1][2], texU2, 0.0f, b->Light[0] * light2, b->Light[1] * light2, b->Light[2] * light2, 1.0f });
					// Vertex 4 (p1[j+1])
					vao.push_back({ b->p1[j + 1][0], b->p1[j + 1][1], b->p1[j + 1][2], texU2, 1.0f, b->Light[0] * light2, b->Light[1] * light2, b->Light[2] * light2, 1.0f });
				}

				// 2. Setup Shader and Uniforms
				myShader.setMat4(g_uMvpLoc, projectionStack.back() * modelViewStack.back());
				//myShader.setBool(g_uTexEnabledLoc, true);

				// 3. Set Attributes
				glEnableVertexAttribArray(g_aPosLoc);
				glVertexAttribPointer(g_aPosLoc, 3, GL_FLOAT, GL_FALSE, sizeof(SpriteVertexFull), &vao[0].x);

				glEnableVertexAttribArray(g_aTexLoc);
				glVertexAttribPointer(g_aTexLoc, 2, GL_FLOAT, GL_FALSE, sizeof(SpriteVertexFull), &vao[0].u);

				glEnableVertexAttribArray(g_aColorLoc);
				glVertexAttribPointer(g_aColorLoc, 4, GL_FLOAT, GL_FALSE, sizeof(SpriteVertexFull), &vao[0].r);

				// 4. Draw all quads at once
				// Since we used GL_TRIANGLE_FAN for 4 vertices per quad, we draw them in batches of 4
				for (int i = 0; i < (b->Number - 1); i++) {
					glDrawArrays(GL_TRIANGLE_FAN, i * 4, 4);
				}

				// 5. Cleanup
				glDisableVertexAttribArray(g_aTexLoc);
				glDisableVertexAttribArray(g_aColorLoc);

			}
		}
	}
	RenderObjectBlurs();
}

typedef struct
{
	bool      Live;
	int       Type;
	int       LifeTime;
	OBJECT *  Owner;
	int       Number;
	vec3_t    Light;
	int		  _iLimitLifeTime;
	vec3_t    p1[MAX_OBJECTBLUR_TAILS];
	vec3_t    p2[MAX_OBJECTBLUR_TAILS];
	int       SubType;
} OBJECT_BLUR;

OBJECT_BLUR ObjectBlur[MAX_OBJECTBLURS];

void ClearAllObjectBlurs()
{
	for(int i = 0; i < MAX_OBJECTBLURS; ++i)
	{
		ObjectBlur[i].Live = false;
	}
}

void AddObjectBlur(OBJECT_BLUR *b,vec3_t p1,vec3_t p2,vec3_t Light,int Type)
{
	b->Type = Type;
	VectorCopy(Light,b->Light);

	for(int i=b->Number-1;i>=0;i--)
	{
		VectorCopy(b->p1[i],b->p1[i+1]);
		VectorCopy(b->p2[i],b->p2[i+1]);
	}
	VectorCopy(p1,b->p1[0]);
	VectorCopy(p2,b->p2[0]);
	b->Number++;

	if(b->Number >= MAX_OBJECTBLUR_TAILS-1)
    {
		b->Number = MAX_OBJECTBLUR_TAILS-1;
    }
}

void CreateObjectBlur(OBJECT *Owner,vec3_t p1,vec3_t p2,vec3_t Light,int Type,bool Short,int SubType, int iLimitLifeTime)
{
	for(int i=0;i<MAX_OBJECTBLURS;i++)
	{
		OBJECT_BLUR *b = &ObjectBlur[i];
		if(b->Live && b->Owner==Owner)
		{
			if(SubType > 0 && b->SubType != SubType)
				continue;
			AddObjectBlur(b,p1,p2,Light,Type);
            return;
		}
	}

	for(int i=0;i<MAX_OBJECTBLURS;i++)
	{
		OBJECT_BLUR *b = &ObjectBlur[i];
		if(!b->Live)
		{
			b->Live = true;
			b->Owner = Owner;
            b->Number= 0;
			if( iLimitLifeTime > -1 )
			{
				b->LifeTime = iLimitLifeTime;
				b->_iLimitLifeTime = iLimitLifeTime;
			}
			else 
			{
				b->_iLimitLifeTime = Short? 15 : MAX_OBJECTBLUR_LIFETIME ;
				b->LifeTime = b->_iLimitLifeTime;				
			}
			b->SubType = SubType;

			AddObjectBlur(b,p1,p2,Light,Type);
            return;
		}
	}
}

void MoveObjectBlurs()
{
	for(int i=0;i<MAX_OBJECTBLURS;i++)
	{
		OBJECT_BLUR *b = &ObjectBlur[i];
		if(b->Live)
		{
			b->LifeTime--;
			b->Number--;

			if(b->LifeTime <= 0)
			{
				b->Number = 0;
				b->Live = false;
				continue;
			}

	        for(int i=b->Number-1;i>=0;i--)
	        {
		        VectorCopy(b->p1[i],b->p1[i+1]);
		        VectorCopy(b->p2[i],b->p2[i+1]);
	        }
		}
	}
}

void RenderObjectBlurs()
{
    int Type;
	for(int i=0;i<MAX_OBJECTBLURS;i++)
	{
		OBJECT_BLUR *b = &ObjectBlur[i];
		if(b->Live)
		{
            Type = b->Type;
			int nTexture = BITMAP_BLUR+Type;
			if(Type == 3)
			{
				nTexture = BITMAP_BLUR2;
			}
			else if(Type == 4)
			{
				nTexture = BITMAP_BLUR;
			}
			else if (Type == 5)
			{
				nTexture = BITMAP_LAVA;
			}

            EnableAlphaBlend();

            if( Type>2 ) Type = Type - 3;
			if(b->Number >= 2)
			{
				BindTexture(nTexture);

				// Use your Full Vertex struct (9 floats: XYZ, UV, RGBA)
				std::vector<SpriteVertexFull> vao;
				// Pre-allocate space for performance (Max 4 vertices per segment)
				vao.reserve((b->Number - 1) * 4);

				for (int j = 0; j < b->Number - 1; j++)
				{
					// 1. Distance Check (Skip quads that are stretched too far)
					float Data = 300.f;
					if (b->SubType == 113 || b->SubType == 114)
					{
						if (abs(b->p1[j][0] - b->p1[j + 1][0]) > Data || abs(b->p1[j][1] - b->p1[j + 1][1]) > Data || abs(b->p1[j][2] - b->p1[j + 1][2]) > Data ||
							abs(b->p1[j][0] - b->p2[j + 1][0]) > Data || abs(b->p1[j][1] - b->p2[j + 1][1]) > Data || abs(b->p2[j][2] - b->p2[j + 1][2]) > Data)
							continue;
					}

					// 2. Calculate Lighting and UVs
					float light1 = (float)(b->Number - j) / (float)b->Number;
					float light2 = (float)(b->Number - (j + 1)) / (float)b->Number;
					float texU1 = (float)j / (float)b->Number;
					float texU2 = (float)(j + 1) / (float)b->Number;

					// 3. Pack 4 vertices for this quad
					// Vertex 0: p1[j]
					vao.push_back({ b->p1[j][0], b->p1[j][1], b->p1[j][2], texU1, 1.0f, b->Light[0] * light1, b->Light[1] * light1, b->Light[2] * light1, 1.0f });
					// Vertex 1: p2[j]
					vao.push_back({ b->p2[j][0], b->p2[j][1], b->p2[j][2], texU1, 0.0f, b->Light[0] * light1, b->Light[1] * light1, b->Light[2] * light1, 1.0f });
					// Vertex 2: p2[j+1]
					vao.push_back({ b->p2[j + 1][0], b->p2[j + 1][1], b->p2[j + 1][2], texU2, 0.0f, b->Light[0] * light2, b->Light[1] * light2, b->Light[2] * light2, 1.0f });
					// Vertex 3: p1[j+1]
					vao.push_back({ b->p1[j + 1][0], b->p1[j + 1][1], b->p1[j + 1][2], texU2, 1.0f, b->Light[0] * light2, b->Light[1] * light2, b->Light[2] * light2, 1.0f });
				}

				if (!vao.empty())
				{
					// 4. Set Shader State
					myShader.use();
					myShader.setMat4(g_uMvpLoc, projectionStack.back() * modelViewStack.back());
					myShader.setBool(g_uTexEnabledLoc, true);
					myShader.setVec4(g_uColorLoc, 1.0f, 1.0f, 1.0f, 1.0f); // Reset global tint to white

					// 5. Attributes
					glEnableVertexAttribArray(g_aPosLoc);
					glVertexAttribPointer(g_aPosLoc, 3, GL_FLOAT, GL_FALSE, sizeof(SpriteVertexFull), &vao[0].x);

					glEnableVertexAttribArray(g_aTexLoc);
					glVertexAttribPointer(g_aTexLoc, 2, GL_FLOAT, GL_FALSE, sizeof(SpriteVertexFull), &vao[0].u);

					glEnableVertexAttribArray(g_aColorLoc);
					glVertexAttribPointer(g_aColorLoc, 4, GL_FLOAT, GL_FALSE, sizeof(SpriteVertexFull), &vao[0].r);

					// 6. Draw the quads in batches of 4
					for (int i = 0; i < vao.size() / 4; i++) {
						glDrawArrays(GL_TRIANGLE_FAN, i * 4, 4);
					}

					// 7. Cleanup
					glDisableVertexAttribArray(g_aTexLoc);
					glDisableVertexAttribArray(g_aColorLoc);
				}

			}
		}
	}
}

void RemoveObjectBlurs(OBJECT *Owner,int SubType)
{
	for(int i=0;i<MAX_OBJECTBLURS;i++)
	{
		OBJECT_BLUR *b = &ObjectBlur[i];
		if(b->Live && b->Owner==Owner)
		{
			ObjectBlur[i].Live = false;
		}
	}
}

void CreateSpark(int Type,CHARACTER *tc,vec3_t Position,vec3_t Angle)
{
	OBJECT *to = &tc->Object;
	vec3_t Light;
	Vector(1.f,1.f,1.f,Light);
   	CreateParticle(BITMAP_SPARK+1,Position,to->Angle,Light);
	vec3_t p,p2;
	float Matrix[3][4];
	Vector(0.f,50.f,0.f,p);
	AngleMatrix(Angle,Matrix);
	VectorRotate(p,Matrix,p2);
	VectorAdd(p2,Position,p2);
	for(int i=0;i<20;i++)
	{
		vec3_t a;
		Vector((float)(rand()%360),0.f,(float)(rand()%360),a);
		VectorAdd(a,Angle,a);
   		//CreateJoint(BITMAP_JOINT_SPARK,p2,p2,a);
	    CreateParticle(BITMAP_SPARK,Position,to->Angle,Light);
	}
}

void CreateBlood(OBJECT *o)
{
	int BoneHead = Models[o->Type].BoneHead;
	if(BoneHead!=-1)
	{
		if(o->Type==MODEL_MONSTER01+15)
		{
			o->Live = false;
			for(int i=0;i<10;i++)
				CreateEffect(MODEL_ICE_SMALL,o->Position,o->Angle,o->Light);
		}
		else if(o->Type!=MODEL_MONSTER01+7 && o->Type!=MODEL_MONSTER01+14 && o->Type!=MODEL_MONSTER01+18)
		{
			vec3_t p,Position;
			for(int i=0;i<2;i++)
			{
				Vector((float)(rand()%100-50),(float)(rand()%100-50),0.f,p);
				Models[o->Type].TransformPosition(o->BoneTransform[BoneHead],p,Position,true);
				CreatePointer(BITMAP_BLOOD,Position,(float)(rand()%360),o->Light,(float)(rand()%4+8)*0.1f);
			}
		}
	}
}

#define FLAG_WIDTH  7
#define FLAG_HEIGHT 10
#define FLAG_SCALE  10.f

typedef struct {
    int link_num;
	int link[4];
	float link_length[4];
	vec3_t p;
	vec3_t v;
	vec3_t f;
	vec3_t uf;
	vec3_t normal;
	float  light;
	bool collision;
} physics_vertex;

typedef struct {
	int vlist[4];
	vec3_t normal;
} physics_face;

typedef struct {
	physics_vertex vtx[8];
} physics_boundbox;

physics_vertex flag_vertex[FLAG_HEIGHT*FLAG_WIDTH];
physics_face   flag_face[(FLAG_HEIGHT-1)*(FLAG_WIDTH-1)];

vec3_t Gravity;
float Damping = 0.04f;
float Ks = 5.f; //hook's spring
float Kd = 0.1f; //spring dumping
float Kr = 0.8f;
float DeltaTime = 0.1f;

void CreateFlag()
{
	for(int i=0;i<FLAG_HEIGHT;i++)
	{
		for(int j=0;j<FLAG_WIDTH;j++)
		{
			int index = i*FLAG_WIDTH+j;
			physics_vertex *v = &flag_vertex[index];
			v->p[0] = -(float)i*FLAG_SCALE;
			v->p[1] = 0.f;
			v->p[2] = (float)j*FLAG_SCALE;
			if(i==0)
			{
				if(j==0)
				{
					v->p[0] -= 5.f;
					v->p[2] += 5.f;
				}
				if(j==6)
				{
					v->p[0] -= 5.f;
					v->p[2] -= 5.f;
				}
				v->p[1] += 10.f;
			}
			v->link_num = 0;
			if(j-1 >= 0            ) {v->link[v->link_num] = (i  )*FLAG_WIDTH+(j-1);v->link_num++;}
			if(j+1 <= FLAG_WIDTH -1) {v->link[v->link_num] = (i  )*FLAG_WIDTH+(j+1);v->link_num++;}
			if(i-1 >= 0            ) {v->link[v->link_num] = (i-1)*FLAG_WIDTH+(j  );v->link_num++;}
			if(i+1 <= FLAG_HEIGHT-1) {v->link[v->link_num] = (i+1)*FLAG_WIDTH+(j  );v->link_num++;}
		}
	}

	for(int i=0;i<FLAG_HEIGHT-1;i++)
	{
		for(int j=0;j<FLAG_WIDTH-1;j++)
		{
			physics_face *f = &flag_face[i*(FLAG_WIDTH-1)+j];
			int index = i*FLAG_WIDTH+j;
			f->vlist[0] = index;
			f->vlist[1] = index+FLAG_WIDTH;
			f->vlist[2] = index+FLAG_WIDTH+1;
			f->vlist[3] = index+1;
		}
	}
}

bool InitFlag = false;
int  wind_frame=0;


void AnimationFlag()
{
	if(!InitFlag)
	{
		InitFlag = true;
		CreateFlag();
	}
	wind_frame++;

	Gravity[0] = -0.2f;
	Gravity[1] = 0.f;
	Gravity[2] = 0.f;

	
	for(int i=2;i<FLAG_HEIGHT;i++)
	{
		for (int j = 0; j < FLAG_WIDTH; ++j)
		{
			int index = i*FLAG_WIDTH+j;
			physics_vertex *v = &flag_vertex[index];

			v->f[0]=0.f;
			v->f[1]=0.f;
			v->f[2]=0.f;
			
			VectorAdd(v->f,Gravity,v->f);
			VectorMA(v->f,-Damping,v->v,v->f);

			//v->f[2] += (float)sin(j*0.4f+wind_frame*0.03f)*(j*0.08f);
			if(i<5)
			{
				if(wind_frame%128<64)
				{
					v->f[1] += 1.f;//-(float)sin(i*10.f+(j-3)*1.f+wind_frame*0.06f)*0.5f;
					v->f[0] -= 1.f;//-(float)sin(i*10.f+(j-3)*1.f+wind_frame*0.06f)*0.5f;
				}
			}
			if(i==9)
			{
				if(wind_frame%128<64)
				{
					v->f[1] -= 0.8f-(float)sin(i*10.f+(j-3)*1.f+wind_frame*0.06f)*0.8f;
				}
			}
			
			for(int k=0;k<v->link_num;k++)
			{
				physics_vertex *v1 = &flag_vertex[index];
				physics_vertex *v2 = &flag_vertex[v->link[k]];
				
				vec3_t DeltaP;
				VectorSubtract(v1->p,v2->p,DeltaP);
				float dist = VectorLength(DeltaP);
				float Hterm = (dist - FLAG_SCALE) * Ks;	
				
				vec3_t DeltaV;
				VectorSubtract(v1->v,v2->v,DeltaV);
				float Dterm = (DotProduct(DeltaV,DeltaP) * Kd) / dist;
				
				vec3_t SpringForce;
				VectorScale(DeltaP,1.f/dist,SpringForce);	
				VectorScale(SpringForce,-(Hterm+Dterm),SpringForce);
				VectorAdd(v1->f,SpringForce,v1->f);	
				if ( (index+1)/FLAG_WIDTH > (v->link[k]+1)/FLAG_WIDTH)
				{
					VectorSubtract(v2->f,SpringForce,v2->f);
				}
			}
		}
	}

	for(int i=2;i<FLAG_HEIGHT;i++)
	{
		for(int j=0;j<FLAG_WIDTH;j++)
		{
			int index = i*FLAG_WIDTH+j;
			physics_vertex *v = &flag_vertex[index];
			VectorMA(v->v,DeltaTime,v->f,v->v);
			VectorMA(v->p,DeltaTime,v->v,v->p);
		}
	}

}

void RenderFlagFace(OBJECT *o,int x,int y,vec3_t Light,int Tex1,int Tex2)
{
    int n=4;
    float su = (float)x/6.f;
    float sv = (float)y/9.f;
	float TexCoord[4][2];
	TexCoord[0][0] = su;
	TexCoord[0][1] = sv;
	TexCoord[1][0] = su;
	TexCoord[1][1] = sv+1.f/9.f;
	TexCoord[2][0] = su+1.f/6.f;
	TexCoord[2][1] = sv+1.f/9.f;
	TexCoord[3][0] = su+1.f/6.f;
	TexCoord[3][1] = sv;

    float minz = 65536.f; //louis
	physics_face *f = &flag_face[y*(FLAG_WIDTH-1)+x];

	for(int i=0;i<n;i++)
	{
		int vlist = f->vlist[i];
     	physics_vertex *v = &flag_vertex[vlist];
		v->light = (-v->normal[0]+v->normal[1])*0.5f+0.5f;
	}

	// Tex2 pass
	BindTexture(Tex2);

	{
		std::vector<MU3DColorVertex> verts;
		verts.reserve(n);

		GLfloat oldColor[4];
		MU_glGetColor4(GL_CURRENT_COLOR, oldColor);

		for (int i = 0; i < n; i++)
		{
			int vlist = f->vlist[i];
			physics_vertex* v = &flag_vertex[vlist];

			vec3_t p, Position;
			Vector(v->p[0] + 9.f, v->p[1] - 12.f, v->p[2] - 35.f, p);
			Models[o->Type].TransformPosition(o->BoneTransform[19], p, Position, true);

			MU3DColorVertex vx = {};

			vx.x = Position[0];
			vx.y = Position[1];
			vx.z = Position[2];

			vx.u = TexCoord[i][0];
			vx.v = TexCoord[i][1];

			vx.r = MU_FloatToColorByte(Light[0] * v->light);
			vx.g = MU_FloatToColorByte(Light[1] * v->light);
			vx.b = MU_FloatToColorByte(Light[2] * v->light);
			vx.a = MU_FloatToColorByte(oldColor[3]); // glColor3f preserves alpha

			verts.push_back(vx);
		}

		if (!verts.empty())
		{
			glUseProgram(g_muProgram);
			MU_ApplyMatrices();

			if (g_uUseTexture >= 0)
				glUniform1i(g_uUseTexture, 1);

			// texture must already be bound before this block
			// glActiveTexture(GL_TEXTURE0);
			// BindTexture(textureId);

			glEnableVertexAttribArray(0);
			glVertexAttribPointer(
				0,
				3,
				GL_FLOAT,
				GL_FALSE,
				sizeof(MU3DColorVertex),
				&verts[0].x
			);

			glEnableVertexAttribArray(1);
			glVertexAttribPointer(
				1,
				2,
				GL_FLOAT,
				GL_FALSE,
				sizeof(MU3DColorVertex),
				&verts[0].u
			);

			glEnableVertexAttribArray(2);
			glVertexAttribPointer(
				2,
				4,
				GL_UNSIGNED_BYTE,
				GL_TRUE,
				sizeof(MU3DColorVertex),
				&verts[0].r
			);

			glDrawArrays(GL_TRIANGLE_FAN, 0, (GLsizei)verts.size());

			glDisableVertexAttribArray(2);
			glDisableVertexAttribArray(1);
			glDisableVertexAttribArray(0);
		}

		glColor4fv(oldColor);
	}


	// Tex1 pass
	BindTexture(Tex1);

	{
		std::vector<MU3DColorVertex> verts;
		verts.reserve(n);

		GLfloat oldColor[4];
		MU_glGetColor4(GL_CURRENT_COLOR, oldColor);

		for (int i = n - 1; i >= 0; i--)
		{
			int vlist = f->vlist[i];
			physics_vertex* v = &flag_vertex[vlist];

			vec3_t p, Position;
			Vector(v->p[0] + 9.f, v->p[1] - 12.f, v->p[2] - 35.f, p);
			Models[o->Type].TransformPosition(o->BoneTransform[19], p, Position, true);

			MU3DColorVertex vx = {};

			vx.x = Position[0];
			vx.y = Position[1];
			vx.z = Position[2];

			vx.u = TexCoord[i][0];
			vx.v = TexCoord[i][1];

			vx.r = MU_FloatToColorByte(Light[0] * v->light);
			vx.g = MU_FloatToColorByte(Light[1] * v->light);
			vx.b = MU_FloatToColorByte(Light[2] * v->light);
			vx.a = MU_FloatToColorByte(oldColor[3]); // glColor3f preserves alpha

			verts.push_back(vx);
		}

		if (!verts.empty())
		{
			glUseProgram(g_muProgram);
			MU_ApplyMatrices();

			if (g_uUseTexture >= 0)
				glUniform1i(g_uUseTexture, 1);

			// texture must already be bound before this draw
			// glActiveTexture(GL_TEXTURE0);
			// BindTexture(textureId);

			glEnableVertexAttribArray(0);
			glVertexAttribPointer(
				0,
				3,
				GL_FLOAT,
				GL_FALSE,
				sizeof(MU3DColorVertex),
				&verts[0].x
			);

			glEnableVertexAttribArray(1);
			glVertexAttribPointer(
				1,
				2,
				GL_FLOAT,
				GL_FALSE,
				sizeof(MU3DColorVertex),
				&verts[0].u
			);

			glEnableVertexAttribArray(2);
			glVertexAttribPointer(
				2,
				4,
				GL_UNSIGNED_BYTE,
				GL_TRUE,
				sizeof(MU3DColorVertex),
				&verts[0].r
			);

			glDrawArrays(GL_TRIANGLE_FAN, 0, (GLsizei)verts.size());

			glDisableVertexAttribArray(2);
			glDisableVertexAttribArray(1);
			glDisableVertexAttribArray(0);
		}

		glColor4fv(oldColor);
	}
}

void RenderFlag(OBJECT *o,vec3_t Light,int Tex1,int Tex2)
{

    for(int y=0;y<FLAG_HEIGHT-1;y++)
	{
		for(int x=0;x<FLAG_WIDTH-1;x++)
        {
         	physics_face *f = &flag_face[y*(FLAG_WIDTH-1)+x];
            FaceNormalize(flag_vertex[f->vlist[0]].p,
				flag_vertex[f->vlist[1]].p,
				flag_vertex[f->vlist[2]].p,
				f->normal);
        }
	}
    for(int y=0;y<FLAG_HEIGHT;y++)
	{
		for(int x=0;x<FLAG_WIDTH;x++)
        {
         	physics_vertex *v = &flag_vertex[y*FLAG_WIDTH+x];
			float num = 0.f;
			if(y<FLAG_HEIGHT-1 && x<FLAG_WIDTH-1) 
			{
				VectorCopy(flag_face[y*(FLAG_WIDTH-1)+x].normal,v->normal);
			}
			else 
			{
				v->normal[0] = 0.f;
				v->normal[1] = 0.f;
				v->normal[2] = 0.f;
			}
        }
	}

    for(int y=0;y<FLAG_HEIGHT-1;y++)
	{
		for(int x=0;x<FLAG_WIDTH-1;x++)
        {
			RenderFlagFace(o,x,y,Light,Tex1,Tex2);
        }
	}
}
