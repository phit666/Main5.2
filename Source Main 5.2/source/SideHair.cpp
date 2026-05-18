// SideHair.cpp: implementation of the CSideHair
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ZzzOpenglUtil.h"
#include "ZzzBmd.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "ShadowVolume.h"
#include "ZzzLodTerrain.h"
#include "zzzTexture.h"
#include "SideHair.h"
#include "ZzzCharacter.h"
#include "wt.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSideHair::CSideHair()
{
	CShadowVolume();
}

CSideHair::~CSideHair()
{

}

void CSideHair::Create( vec3_t ppVertexTransformed[MAX_MESH][MAX_VERTICES], BMD *b, OBJECT *o, bool SkipTga)
{
	VectorSubtract( Hero->Object.Position, CameraPosition, m_vLight);
	VectorNormalize( m_vLight);

	if ( o->Alpha < 0.01f)
	{
		return;
	}
	short nHiddenMesh = o->HiddenMesh;
	short nBlendMesh = o->BlendMesh;
	if ( nHiddenMesh == -2 || nBlendMesh == -2)
	{
		return;
	}

	int iNumTriangles = 0;
	for ( int i = 1; i < 2; ++i)
	{
		if ( nHiddenMesh == i || nBlendMesh == i)
		{
			continue;
		}
		if(Bitmaps[b->IndexTexture[i]].Components == 4)
		{
     		if(SkipTga) continue;
		}
		iNumTriangles += b->Meshs[i].NumTriangles;
	}
	m_iNumEdge = 0;
	m_pEdges = new St_Edges[iNumTriangles*3];

	for ( int i = 1; i < 2; ++i)
	{
		if ( nHiddenMesh == i || nBlendMesh == i)
		{
			continue;
		}

		bool Tga = false;
		if(Bitmaps[b->IndexTexture[i]].Components == 4)
		{
			Tga = true;
     		if(SkipTga) continue;
		}
		DeterminateSilhouette( i, ppVertexTransformed, b->Meshs[i].NumTriangles, b->Meshs[i].Triangles,Tga);
	}
}

void CSideHair::Destroy( void)
{
	if ( m_pEdges)
	{
		delete [] m_pEdges;
	}
	if ( m_pVertices)
	{
		delete [] m_pVertices;
	}
}

void CSideHair::Render( vec3_t ppVertexTransformed[MAX_MESH][MAX_VERTICES], vec3_t ppLightTransformed[MAX_MESH][MAX_VERTICES])
{
	for ( int i = 0; i < m_iNumEdge; ++i)
	{
		RenderLine( ppVertexTransformed[m_pEdges[i].m_nMesh][m_pEdges[i].m_nVertexIndex[0]],
			ppVertexTransformed[m_pEdges[i].m_nMesh][m_pEdges[i].m_nVertexIndex[1]],
			ppLightTransformed[m_pEdges[i].m_nMesh][m_pEdges[i].m_nNormalIndex[0]],
			ppLightTransformed[m_pEdges[i].m_nMesh][m_pEdges[i].m_nNormalIndex[1]]);
	}
}

void CSideHair::RenderLine( vec3_t v1, vec3_t v2, vec3_t c1, vec3_t c2)
{
	vec3_t p1, p2, d;

	glColor3f(1.f, 1.f, 1.f);
	VectorSubtract(v2, v1, d);
	float fLength = VectorLength(d);
	float fTextureMove = 0.0f;
	fTextureMove = (50.0f - fLength) * 0.5f / 50.0f;

	VectorCopy(v1, p1);
	VectorCopy(v2, p2);
	VectorSubtract(p2, p1, d);
	VectorScale(d, 0.1f, d);
	VectorSubtract(p1, d, p1);
	VectorAdd(p2, d, p2);

	float fTextureV = (float)(rand() % 100) * 0.01f;
	glColor3f(1.f, 1.f, 1.f);
	BindTexture(BITMAP_ROBE + 4);
	EnableAlphaBlendMinus();
	//EnableAlphaTest();
	//g_OpenglLib.DisableTexture();
	//g_OpenglLib.Disable(GL_CULL_FACE);
	/*glBegin(GL_QUADS);
	glTexCoord2f(0.f,0.f+fTextureMove);glVertex3f(p1[0]-Scale,p1[1],p1[2]);
	glTexCoord2f(0.f,1.f-fTextureMove);glVertex3f(p2[0]-Scale,p2[1],p2[2]);
	glTexCoord2f(1.f,1.f-fTextureMove);glVertex3f(p2[0]+Scale,p2[1],p2[2]);
	glTexCoord2f(1.f,0.f+fTextureMove);glVertex3f(p1[0]+Scale,p1[1],p1[2]);
	glEnd();
	glBegin(GL_QUADS);
	glTexCoord2f(0.f,0.f+fTextureMove);glVertex3f(p1[0],p1[1]-Scale,p1[2]);
	glTexCoord2f(0.f,1.f-fTextureMove);glVertex3f(p2[0],p2[1]-Scale,p2[2]);
	glTexCoord2f(1.f,1.f-fTextureMove);glVertex3f(p2[0],p2[1]+Scale,p2[2]);
	glTexCoord2f(1.f,0.f+fTextureMove);glVertex3f(p1[0],p1[1]+Scale,p1[2]);
	glEnd();
	glBegin(GL_QUADS);
	glTexCoord2f(0.f,0.f+fTextureMove);glVertex3f(p1[0],p1[1],p1[2]-Scale);
	glTexCoord2f(0.f,1.f-fTextureMove);glVertex3f(p2[0],p2[1],p2[2]-Scale);
	glTexCoord2f(1.f,1.f-fTextureMove);glVertex3f(p2[0],p2[1],p2[2]+Scale);
	glTexCoord2f(1.f,0.f+fTextureMove);glVertex3f(p1[0],p1[1],p1[2]+Scale);
	glEnd();*/
	vec3_t vOrtho;
	CrossProduct(m_vLight, d, vOrtho);
	VectorNormalize(vOrtho);
	VectorScale(vOrtho, 10.f, vOrtho);

	// 1. Pack data into the 3D vertex struct
	SpriteVertex3D vao[4];

	// Using unique names for texture coordinates to avoid conflict with parameter v1/v2
	float texV_Start = 0.0f + fTextureMove + fTextureV;
	float texV_End = 1.0f - fTextureMove + fTextureV;

	// Vertex 0
	vao[0].x = p1[0] - vOrtho[0];
	vao[0].y = p1[1] - vOrtho[1];
	vao[0].z = p1[2] - vOrtho[2];
	vao[0].u = 0.0f;
	vao[0].v = texV_Start;

	// Vertex 1
	vao[1].x = p2[0] - vOrtho[0];
	vao[1].y = p2[1] - vOrtho[1];
	vao[1].z = p2[2] - vOrtho[2];
	vao[1].u = 0.0f;
	vao[1].v = texV_End;

	// Vertex 2
	vao[2].x = p2[0] + vOrtho[0];
	vao[2].y = p2[1] + vOrtho[1];
	vao[2].z = p2[2] + vOrtho[2];
	vao[2].u = 1.0f;
	vao[2].v = texV_End;

	// Vertex 3
	vao[3].x = p1[0] + vOrtho[0];
	vao[3].y = p1[1] + vOrtho[1];
	vao[3].z = p1[2] + vOrtho[2];
	vao[3].u = 1.0f;
	vao[3].v = texV_Start;

	glBindBuffer(GL_ARRAY_BUFFER, g_meshVBO);

	glBufferData(
		GL_ARRAY_BUFFER,
		sizeof(vao),
		vao,
		GL_STREAM_DRAW
	);

	safe_enable_attr(g_aPosLoc);
	glVertexAttribPointer(
		g_aPosLoc,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(SpriteVertex3D),
		(void*)offsetof(SpriteVertex3D, x)
	);

	safe_enable_attr(g_aTexLoc);
	glVertexAttribPointer(
		g_aTexLoc,
		2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(SpriteVertex3D),
		(void*)offsetof(SpriteVertex3D, u)
	);

	safe_disable_attr(g_aColorLoc);

	MU_ApplyMatrices();

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glDisableVertexAttribArray(g_aTexLoc);
	glDisableVertexAttribArray(g_aPosLoc);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

}
