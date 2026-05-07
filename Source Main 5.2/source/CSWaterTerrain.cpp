//////////////////////////////////////////////////////////////////////////
//  CSWaterTerrain.cpp
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "ZzzOpenglUtil.h"
#include "zzzInfomation.h"
#include "zzzBmd.h"
#include "zzzObject.h"
#include "zzztexture.h"
#include "zzzCharacter.h"
#include "zzzscene.h"
#include "zzzInterface.h"
#include "zzzinventory.h"
#include "zzzLodTerrain.h"
#include "CSWaterTerrain.h"
#include "GMHellas.h"
#include "MapManager.h"
#include "mu_gles2_matrix.h"

extern  float   WorldTime;
extern  int     MoveSceneFrame;
extern  float   TerrainMappingAlpha[TERRAIN_SIZE*TERRAIN_SIZE];
extern  float   g_chrome[MAX_VERTICES][2];

void CSWaterTerrain::Init ( void )
{
    Vector ( 1.f, -1.f, 1.f, m_vLightVector );

    memset ( m_iWaveHeight, 0, sizeof( int )*WATER_TERRAIN_SIZE*WATER_TERRAIN_SIZE*4 );
}

void CSWaterTerrain::Update ( void )
{
    if ( !gMapManager.InHellas(m_iMapIndex) ) return;

    int WaveX;
    int WaveY; 
    //int Deep1 = (int)( 1050+sin(WorldTime*0.003f)*100 );

    if ( (MoveSceneFrame%40)==0 )
    {
        WaveX = ((Hero->PositionX)*2)+(rand()%30)-15;
        WaveY = ((Hero->PositionY)*2)+25;
        addSineWave ( WaveX, WaveY, 20, 2, 2000 );
    }

    m_iWaterPage ^= 1;
    calcWave ();
    calcBaseWave ();
}

void    CSWaterTerrain::Render ( void )
{
    if ( !gMapManager.InHellas(m_iMapIndex) ) return;

    CreateTerrain ( (Hero->PositionX)*2, (Hero->PositionY)*2 );

    float alpha;
    int   offset;
    int   i, j;
	for ( i=0; i<MAX_WATER_GRID*MAX_WATER_GRID; i++ )
	{
		float *Normal = m_Normals[i];
		g_chrome[i][0] = Normal[2]*0.5f + 0.1f;
		g_chrome[i][1] = Normal[1]*0.5f + 0.5f;
	}

    EnableAlphaTest ();
	BindTexture ( BITMAP_MAPTILE );


    std::vector<MU3DVertex> verts(m_iTriangleListNum);

    for (j = 0; j < m_iTriangleListNum; j++)
    {
        int offset = m_iTriangleList[j];

        verts[j].x = m_Vertices[offset][0];
        verts[j].y = m_Vertices[offset][1];
        verts[j].z = m_Vertices[offset][2];

        verts[j].u = g_chrome[offset][1];
        verts[j].v = g_chrome[offset][0];
    }


    glUseProgram(g_muProgram);
    MU_ApplyMatrices();

    if (g_uUseTexture >= 0)
        glUniform1i(g_uUseTexture, 1);

    // texture should already be bound before this draw
    // If not, bind it BEFORE the attributes:
    // glActiveTexture(GL_TEXTURE0);
    // BindTexture(textureId);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(MU3DVertex),
        &verts[0].x
    );

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(MU3DVertex),
        &verts[0].u
    );

    glColor3f(0.2f, 0.5f, 0.65f);

    glDrawArrays(GL_TRIANGLES, 0, m_iTriangleListNum);

    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);


    EnableAlphaBlend ();
	BindTexture ( BITMAP_MAPTILE+1 );


    std::vector<MU3DColorVertex> colorverts(m_iTriangleListNum);

    for (j = 0; j < m_iTriangleListNum; j++)
    {
        int offset = m_iTriangleList[j];

        float alpha = 1.f - DotProduct(m_Normals[offset], m_vLightVector);

        // clamp before converting
        float r = alpha;
        float g = alpha * 2.5f;
        float b = alpha * 3.f;

        if (r < 0.f) r = 0.f; if (r > 1.f) r = 1.f;
        if (g < 0.f) g = 0.f; if (g > 1.f) g = 1.f;
        if (b < 0.f) b = 0.f; if (b > 1.f) b = 1.f;

        colorverts[j].x = m_Vertices[offset][0];
        colorverts[j].y = m_Vertices[offset][1];
        colorverts[j].z = m_Vertices[offset][2];

        colorverts[j].u = g_chrome[offset][1];
        colorverts[j].v = g_chrome[offset][0];

        colorverts[j].r = (GLubyte)(r * 255.f);
        colorverts[j].g = (GLubyte)(g * 255.f);
        colorverts[j].b = (GLubyte)(b * 255.f);
        colorverts[j].a = 255;
    }

    glUseProgram(g_muProgram);
    MU_ApplyMatrices();

    if (g_uUseTexture >= 0)
        glUniform1i(g_uUseTexture, 1);

    // texture should already be bound before this draw
    // glActiveTexture(GL_TEXTURE0);
    // BindTexture(textureId);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(MU3DColorVertex),
        &colorverts[0].x
    );

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(MU3DColorVertex),
        &colorverts[0].u
    );

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2,
        4,
        GL_UNSIGNED_BYTE,
        GL_TRUE,
        sizeof(MU3DColorVertex),
        &colorverts[0].r
    );

    glDrawArrays(GL_TRIANGLES, 0, m_iTriangleListNum);

    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);
}

void    CSWaterTerrain::CreateTerrain ( int x, int y )
{
    float   fHeight, fHeight1;
    int     offset;
    int     grid = MAX_WATER_GRID/2;

    m_iTriangleListNum = 0;
    for ( int offY=0, i=y-grid+6; offY<MAX_WATER_GRID; ++i, offY++ )
    {
        for ( int offX=0, j=x-grid-4; offX<MAX_WATER_GRID; ++j, offX++ )
        {
            if ( i<0 || j<0 ) fHeight = 350.f;
            else if ( i>=WATER_TERRAIN_SIZE || j>=WATER_TERRAIN_SIZE ) fHeight = 350.f;
            else
            {
                offset = j+(i*WATER_TERRAIN_SIZE);

                fHeight = m_iWaveHeight[m_iWaterPage][offset]+350.f;

                fHeight1 = m_iWaveHeight[2][offset]+350.f;
                fHeight1+= m_iWaveHeight[3][offset]+350.f;

                fHeight = (fHeight+fHeight1/2.f)/2.f;
            }
        
            offset = offX+(offY*MAX_WATER_GRID);
            Vector ( (float)j*WAVE_SCALE, (float)i*WAVE_SCALE, fHeight, m_Vertices[offset] );
            Vector ( 0.f, 0.f, 0.f, m_Normals[offset] );
            if ( offX>=MAX_WATER_GRID-1 || offY>=MAX_WATER_GRID-1 )
            {
                VectorCopy( m_Normals[offset-1], m_Normals[offset] );
                continue;
            }

            if ( ( (offX%2)==0 && (offY%2)==0 ) || ( (offX%2)==1 && (offY%2)==1 ) )
            {
                m_iTriangleList[m_iTriangleListNum+0] = offset;
                m_iTriangleList[m_iTriangleListNum+1] = offset+1;
                m_iTriangleList[m_iTriangleListNum+2] = offset+1+MAX_WATER_GRID;
  
                m_iTriangleList[m_iTriangleListNum+3] = offset;
                m_iTriangleList[m_iTriangleListNum+4] = offset+1+MAX_WATER_GRID;
                m_iTriangleList[m_iTriangleListNum+5] = offset+MAX_WATER_GRID;
            }
            else
            {
                m_iTriangleList[m_iTriangleListNum+0] = offset;
                m_iTriangleList[m_iTriangleListNum+1] = offset+1;
                m_iTriangleList[m_iTriangleListNum+2] = offset+MAX_WATER_GRID;
  
                m_iTriangleList[m_iTriangleListNum+3] = offset+1;
                m_iTriangleList[m_iTriangleListNum+4] = offset+1+MAX_WATER_GRID;
                m_iTriangleList[m_iTriangleListNum+5] = offset+MAX_WATER_GRID;
            }

            m_iTriangleListNum+=6;
        }
    }

    int     v1, v2, v3;
    vec3_t  normalV;
    int     NormalNum[MAX_WATER_GRID*MAX_WATER_GRID] = { 0, };
	for ( int j=0; j<m_iTriangleListNum; j+=3 )
	{
        v1 = m_iTriangleList[j+0];
        v2 = m_iTriangleList[j+1];
        v3 = m_iTriangleList[j+2];

        FaceNormalize ( m_Vertices[v1], m_Vertices[v2], m_Vertices[v3], normalV );

        VectorAdd ( normalV, m_Normals[v1], m_Normals[v1] );
        VectorAdd ( normalV, m_Normals[v2], m_Normals[v2] );
        VectorAdd ( normalV, m_Normals[v3], m_Normals[v3] );

        NormalNum[v1]++;
        NormalNum[v2]++;
        NormalNum[v3]++;
	}

	for (int i=0; i<MAX_WATER_GRID*MAX_WATER_GRID; i++ )
	{
        m_Normals[i][0] /= (float)(NormalNum[i]);
        m_Normals[i][1] /= (float)(NormalNum[i]);
        m_Normals[i][2] /= (float)(NormalNum[i]);

        VectorNormalize ( m_Normals[i] );
	}
}

void CSWaterTerrain::addSineWave ( int x, int y, int radiusX, int radiusY, int height )
{
	int* p = &m_iWaveHeight[m_iWaterPage][0];

	int     cx, cy;
	int     left, top, right, bottom;
	int     square;
	int     radsquare;
	float   length = (1024.f/(float)radiusX)*(1024.f/(float)radiusX);
	
	if ( x<0 ) x = 1+radiusX+ rand()%(WATER_TERRAIN_SIZE-2*radiusX-1);
	if ( y<0 ) y = 1+radiusY+ rand()%(WATER_TERRAIN_SIZE-2*radiusY-1);
	
	//  radsquare = (radiusX*radiusX) << 8;
	radsquare = ( radiusX*radiusY );
	
	//  height /= 8;
	left = -radiusX; right  = radiusX;
	top  = -radiusY; bottom = radiusY;
	
	// Perform edge clipping...
	if ( (x-radiusX)<1 ) left -= ( x-radiusX-1 );
	if ( (y-radiusY)<1 ) top  -= ( y-radiusY-1 );
	if ( (x+radiusX)>WATER_TERRAIN_SIZE-1 ) right -= ( x+radiusX-WATER_TERRAIN_SIZE+1 );
	if ( (y+radiusY)>WATER_TERRAIN_SIZE-1 ) bottom-= ( y+radiusY-WATER_TERRAIN_SIZE+1 );
	
	for ( cy=top; cy<bottom; cy++ )
	{
		for ( cx=left; cx<right; cx++ )
		{
			square = cy*cy + cx*cx;
			if ( square<radsquare )
			{
				float dist = sqrtf ( square*length );
                int   sine = (int)(( cos( dist )+0xffff )*height)>>19;
				p[WATER_TERRAIN_SIZE*(cy+y)+cx+x] += sine;
			}
		}
	}
}

void    CSWaterTerrain::calcBaseWave ( void )
{
/*
    if ( (rand()%10)==0 )
    {
        m_iSelectWaveX = rand()%WATER_TERRAIN_SIZE;
        m_iSelectWaveY = rand()%WATER_TERRAIN_SIZE;
        m_iAddHeight   = rand()%20+10;
    }
*/
    int MaxHeight;
    int offset;
    int HeroX = ( Hero->PositionX )*2;
    int HeroY = ( Hero->PositionY )*2;
/*
    int HeroX = ( Hero->Object.Position[0]/TERRAIN_SCALE )*2;
    int HeroY = ( Hero->Object.Position[1]/TERRAIN_SCALE )*2;
*/

    int StartX = max ( 0, HeroX-(VIEW_WATER_GRID/2) );
    int StartY = max ( 0, HeroY-(VIEW_WATER_GRID/2) );
    int EndX   = min ( WATER_TERRAIN_SIZE, HeroX+(VIEW_WATER_GRID/2) );
    int EndY   = min ( WATER_TERRAIN_SIZE, HeroY+(VIEW_WATER_GRID/2) );
	for ( int i=StartY; i<EndY; i++ )        //  y
    {
        for ( int j=StartX; j<EndX; j++ )    //  x
        {
            offset = j+(i*WATER_TERRAIN_SIZE);

            //  큰 사인곡선
            float alpha = 0.f;//TerrainMappingAlpha[(j/2)+(i/2)*WATER_TERRAIN_SIZE];

            MaxHeight = (int)( sin( (WorldTime*0.005f)+(i*0.1f)+(j*0.1f) )*50*(1+alpha) );
            m_iWaveHeight[2][offset] = (int) ( MaxHeight - sin ( (WorldTime*0.003f)+(j*0.1f)+(i*0.5f) )*50*(1+alpha) );

            //  작은 사인곡선
            MaxHeight = (int)( sin( (WorldTime*0.001f)+(i*0.5f)+(j*0.5f) )*25*(1+alpha) );
            m_iWaveHeight[3][offset] = (int) ( MaxHeight - sin ( (WorldTime*0.002f)+(j*1.f)+(i*0.3f) )*25*(1+alpha) );
        }
    }
}

void CSWaterTerrain::calcWave ( void )
{
	int newh;
	
	int* newptr = &m_iWaveHeight[m_iWaterPage][0];
	int* oldptr = &m_iWaveHeight[m_iWaterPage^1][0];
	
	int x;
    int y = (WATER_TERRAIN_SIZE-1)*WATER_TERRAIN_SIZE;
	for ( int count = WATER_TERRAIN_SIZE+1; count<y; count+=2 )
	{
		for ( x=count+WATER_TERRAIN_SIZE-2; count<x; count++ )
		{
			newh = ( ( oldptr[count+WATER_TERRAIN_SIZE]
				     + oldptr[count-WATER_TERRAIN_SIZE]
				     + oldptr[count+1]
				     + oldptr[count-1]
  			       ) >> 1 )
				     - newptr[count];
			newptr[count] = newh-( newh>>4 );
		}
	}
}

float CSWaterTerrain::GetWaterTerrain ( float xf, float yf )
{
    int x = (int)(xf/TERRAIN_SCALE*2);
    int y = (int)(yf/TERRAIN_SCALE*2);

    float fHeight;
    if ( m_iWaterPage )
    {
        fHeight = m_iWaveHeight[1][x+(y*WATER_TERRAIN_SIZE)]+350.f;
    }
    else
    {
        fHeight = m_iWaveHeight[0][x+(y*WATER_TERRAIN_SIZE)]+350.f;
    }
    
    float fHeight1 = m_iWaveHeight[2][x+(y*WATER_TERRAIN_SIZE)]+350.f;
    fHeight1+= m_iWaveHeight[3][x+(y*WATER_TERRAIN_SIZE)]+350.f;
    fHeight = (fHeight+fHeight1/2.f)/4.f;

    return fHeight;
}

void CSWaterTerrain::RenderWaterAlphaBitmap ( int Texture, float xf, float yf, float SizeX, float SizeY, vec3_t Light, float Rotation, float Alpha, float Height )
{
	if(Alpha==1.f)
     	glColor3fv(Light);
	else
     	glColor4f(Light[0],Light[1],Light[2],Alpha);

	vec3_t Angle;
	Vector(0.f,0.f,Rotation,Angle);
	float Matrix[3][4];
	AngleMatrix(Angle,Matrix);
	
	BindTexture(Texture);
	float mxf = (xf/TERRAIN_SCALE*2);
	float myf = (yf/TERRAIN_SCALE*2);
	int   mxi = (int)(mxf);
	int   myi = (int)(myf);

	float Size;
	if(SizeX >= SizeY)
		Size = SizeX;
	else
		Size = SizeY;
	float TexU = (((float)mxi-mxf)+0.5f*Size);
	float TexV = (((float)myi-myf)+0.5f*Size);
	float TexScaleU = 1.f/Size;
	float TexScaleV = 1.f/Size;
	Size = (float)((int)Size+1);
	float Aspect = SizeX/SizeY;
    for(float y=-Size;y<=Size;y+=1.f)
	{
		for(float x=-Size;x<=Size;x+=1.f)
		{
			vec3_t p1[4],p2[4];
			Vector((TexU+x    )*TexScaleU,(TexV+y    )*TexScaleV,0.f,p1[0]);
			Vector((TexU+x+1.f)*TexScaleU,(TexV+y    )*TexScaleV,0.f,p1[1]);
			Vector((TexU+x+1.f)*TexScaleU,(TexV+y+1.f)*TexScaleV,0.f,p1[2]);
			Vector((TexU+x    )*TexScaleU,(TexV+y+1.f)*TexScaleV,0.f,p1[3]);
			//bool Clip = false;
			for(int i=0;i<4;i++) 
			{
				p1[i][0] -= 0.5f;
				p1[i][1] -= 0.5f;
				VectorRotate(p1[i],Matrix,p2[i]);
				p2[i][0] *= Aspect;
				p2[i][0] += 0.5f;
				p2[i][1] += 0.5f;
				//if((p2[i][0]>=0.f && p2[i][0]<=1.f) || (p2[i][1]>=0.f && p2[i][1]<=1.f)) Clip = true;
			}
			//if(Clip==true)
     			RenderWaterBitmapTile((float)mxi+x,(float)myi+y,1.f,1,p2,false,Alpha,Height);
		}
	}
}

void CSWaterTerrain::RenderWaterBitmapTile(
    float xf, float yf, float lodf, int lodi,
    vec3_t c[4], bool LightEnable, float Alpha, float Height)
{
    vec3_t TerrainVertex[4];
    int xi = (int)xf;
    int yi = (int)yf;
    if (xi < 0 || yi < 0 || xi >= TERRAIN_SIZE_MASK || yi >= TERRAIN_SIZE_MASK) return;
    float TileScale = WAVE_SCALE;
    float sx = xf * WAVE_SCALE;
    float sy = yf * WAVE_SCALE;
    int TerrainIndex1 = xi + (yi * WATER_TERRAIN_SIZE);
    int TerrainIndex2 = xi + lodi + (yi * WATER_TERRAIN_SIZE);
    int TerrainIndex3 = xi + lodi + ((yi + lodi) * WATER_TERRAIN_SIZE);
    int TerrainIndex4 = xi + ((yi + lodi) * WATER_TERRAIN_SIZE);
    Vector(sx, sy, m_iWaveHeight[0][TerrainIndex1] + 400.f + Height, TerrainVertex[0]);
    Vector(sx + TileScale, sy, m_iWaveHeight[0][TerrainIndex2] + 400.f + Height, TerrainVertex[1]);
    Vector(sx + TileScale, sy + TileScale, m_iWaveHeight[0][TerrainIndex3] + 400.f + Height, TerrainVertex[2]);
    Vector(sx, sy + TileScale, m_iWaveHeight[0][TerrainIndex4] + 400.f + Height, TerrainVertex[3]);

    vec3_t Light[4];
    if (LightEnable)
    {
        VectorCopy(PrimaryTerrainLight[TerrainIndex1], Light[0]);
        VectorCopy(PrimaryTerrainLight[TerrainIndex2], Light[1]);
        VectorCopy(PrimaryTerrainLight[TerrainIndex3], Light[2]);
        VectorCopy(PrimaryTerrainLight[TerrainIndex4], Light[3]);
    }

    // 1. Pack the data into the Full Vertex struct (9 floats per vertex)
    SpriteVertexFull vao[4];

    for (int i = 0; i < 4; i++) {
        // Position (XYZ)
        vao[i].x = TerrainVertex[i][0];
        vao[i].y = TerrainVertex[i][1];
        vao[i].z = TerrainVertex[i][2];

        // UVs
        vao[i].u = c[i][0];
        vao[i].v = c[i][1];

        // Color (RGBA)
        if (LightEnable) {
            vao[i].r = Light[i][0];
            vao[i].g = Light[i][1];
            vao[i].b = Light[i][2];
            vao[i].a = Alpha; // Alpha is applied even if it's 1.0f
        }
        else {
            // Default to white if lighting is off
            vao[i].r = 1.0f; vao[i].g = 1.0f; vao[i].b = 1.0f; vao[i].a = 1.0f;
        }
    }

    // 2. Set Uniforms
    myShader.setMat4(g_uMvpLoc, projectionStack.back() * modelViewStack.back());
    myShader.setMat4(g_uMvLoc, modelViewStack.back()); // Needed for Fog distance
    //myShader.setBool(g_uTexEnabledLoc, true);

    // 3. Set Attributes
    // Position
    glEnableVertexAttribArray(g_aPosLoc);
    glVertexAttribPointer(g_aPosLoc, 3, GL_FLOAT, GL_FALSE, sizeof(SpriteVertexFull), &vao[0].x);

    // UV
    glEnableVertexAttribArray(g_aTexLoc);
    glVertexAttribPointer(g_aTexLoc, 2, GL_FLOAT, GL_FALSE, sizeof(SpriteVertexFull), &vao[0].u);

    // Color (Light)
    glEnableVertexAttribArray(g_aColorLoc);
    glVertexAttribPointer(g_aColorLoc, 4, GL_FLOAT, GL_FALSE, sizeof(SpriteVertexFull), &vao[0].r);

    // 4. Draw
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    // 5. Cleanup optional attributes
    glDisableVertexAttribArray(g_aTexLoc);
    glDisableVertexAttribArray(g_aColorLoc);

}
