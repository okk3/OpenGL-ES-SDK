#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include "nativewin.h"
#include "sbm.h"
#include "vecmath.h"

#include <iostream>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>

class RenderState 
{
public:
    RenderState() : po(0), vertLoc(0), mvpLoc(0), normalLoc(0), texcoordLoc(0), texUnitLoc(0)
    {}
    ~RenderState() {}

    GLint po;
    GLint vertLoc;
    GLint mvpLoc;
    GLint lightLoc;
    GLint normalLoc;
    GLint texcoordLoc;
    GLint texUnitLoc;

    GLfloat yaw;
    GLfloat pitch;

    SBObject            ninja;
    GLuint              ninjaTex[1];

};

class esContext
{
public:
    esContext() :
        nativeDisplay(0), nativeWin(0),
        eglDisplay(0), eglSurface(0), eglContext(0), 
        nWindowWidth(0), nWindowHeight(0), nMouseX(0), nMouseY(0)
    {}

    ~esContext() {}

    EGLNativeDisplayType nativeDisplay;
    EGLNativeWindowType nativeWin;
    EGLDisplay eglDisplay;
    EGLSurface eglSurface;
    EGLContext eglContext;

    int         nWindowWidth;
    int         nWindowHeight;
    int         nMouseX;
    int         nMouseY;

    RenderState rs;
};

GLfloat vWhite[] = { 1.0, 1.0, 1.0, 1.0 };

#pragma pack(1)
struct RGB { 
  GLbyte blue;
  GLbyte green;
  GLbyte red;
  GLbyte alpha;
};

struct BMPInfoHeader {
  GLuint	size;
  GLuint	width;
  GLuint	height;
  GLushort  planes;
  GLushort  bits;
  GLuint	compression;
  GLuint	imageSize;
  GLuint	xScale;
  GLuint	yScale;
  GLuint	colors;
  GLuint	importantColors;
};

struct BMPHeader {
  GLushort	type; 
  GLuint	size; 
  GLushort	unused; 
  GLushort	unused2; 
  GLuint	offset; 
}; 

struct BMPInfo {
  BMPInfoHeader		header;
  RGB				colors[1];
};

#pragma pack(8)

esContext ctx;

using namespace std;

void OnNativeWinResize(int width, int height)
{
    ctx.nWindowWidth = width;
    ctx.nWindowHeight = height;
    glViewport(0, 0, width, height);
}

void OnNativeWinMouseMove(int mousex, int mousey, bool lbutton)
{
    if(lbutton)
    {
        int oldx = ctx.nMouseX;
        int oldy = ctx.nMouseY;
        GLfloat yaw = ctx.rs.yaw;
        GLfloat pitch = ctx.rs.pitch;
        yaw   += (oldx - mousex) * 720.0f/ctx.nWindowWidth;
        pitch += (oldy - mousey) * 360.0f/ctx.nWindowHeight;
        yaw = (abs(yaw) <  360) ? yaw : yaw/360;
        pitch = (pitch <  90) ? pitch :  89;
        pitch = (pitch > -90) ? pitch : -89;
        ctx.rs.yaw = yaw;
        ctx.rs.pitch = pitch;
    }
    ctx.nMouseX = mousex;
    ctx.nMouseY = mousey;
}

bool LoadTexture(esContext &  tx)
{
    FILE*	pFile;
	BMPInfo *pBitmapInfo = NULL;
	unsigned long lInfoSize = 0;
	unsigned long lBitSize = 0;
	GLbyte *pBits = NULL;					
	BMPHeader	bitmapHeader;

    pFile = fopen("./ninja/ninjacomp.bmp", "rb");
    if(pFile == NULL)
        return false;

    if(fread(&bitmapHeader, 1, sizeof(BMPHeader), pFile) != sizeof(BMPHeader))
    {
        fclose(pFile);
        printf("Failed to load texture.\n");
		return false;
    }

	lInfoSize = bitmapHeader.offset - sizeof(BMPHeader);
	pBitmapInfo = (BMPInfo *) malloc(sizeof(GLbyte)*lInfoSize);
	if(fread(pBitmapInfo, 1, lInfoSize, pFile) != lInfoSize)
	{
		free(pBitmapInfo);
		fclose(pFile);
        printf("Failed to load texture.\n");
		return false;
	}

	GLint nWidth = pBitmapInfo->header.width;
	GLint nHeight = pBitmapInfo->header.height;
	lBitSize = pBitmapInfo->header.imageSize;

	if(pBitmapInfo->header.bits != 24)
	{
        printf("Failed to load texture.\n");
		free(pBitmapInfo);
		return false;
	}

	if(lBitSize == 0)
		lBitSize = (nWidth *
           pBitmapInfo->header.bits + 7) / 8 *
  		  abs(nHeight);

	free(pBitmapInfo);
	pBits = (GLbyte*)malloc(sizeof(GLbyte)*lBitSize);

	if(fread(pBits, 1, lBitSize, pFile) != lBitSize)
	{
		free(pBits);
		pBits = NULL;
	}

	fclose(pFile);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, nWidth, nHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, pBits);

    return true;
}

EGLBoolean Setup(esContext &ctx)
{
    EGLBoolean bsuccess;

    // create native window
    EGLNativeDisplayType nativeDisplay;
    if(!OpenNativeDisplay(&nativeDisplay))
    {
        printf("Could not get open native display\n");
        return GL_FALSE;
    }

    // get egl display handle
    EGLDisplay eglDisplay;
    eglDisplay = eglGetDisplay(nativeDisplay);
    if(eglDisplay == EGL_NO_DISPLAY)
    {
        printf("Could not get EGL display\n");
        CloseNativeDisplay(nativeDisplay);
        return GL_FALSE;
    }
    ctx.eglDisplay = eglDisplay;

    // Initialize the display
    EGLint major = 0;
    EGLint minor = 0;
    bsuccess = eglInitialize(eglDisplay, &major, &minor);
    if (!bsuccess)
    {
        printf("Could not initialize EGL display\n");
        CloseNativeDisplay(nativeDisplay);
        return GL_FALSE;
    }
    if (major < 1 || minor < 4)
    {
        // Does not support EGL 1.4
        printf("System does not support at least EGL 1.4\n");
        CloseNativeDisplay(nativeDisplay);
        return GL_FALSE;
    }

    // Obtain the first configuration with a depth buffer
    EGLint attrs[] = { EGL_DEPTH_SIZE, 16, EGL_NONE };
    EGLint numConfig =0;
    EGLConfig eglConfig = 0;
    bsuccess = eglChooseConfig(eglDisplay, attrs, &eglConfig, 1, &numConfig);
    if (!bsuccess)
    {
        printf("Could not find valid EGL config\n");
        CloseNativeDisplay(nativeDisplay);
        return GL_FALSE;
    }

    // Get the native visual id
    int nativeVid;
    if (!eglGetConfigAttrib(eglDisplay, eglConfig, EGL_NATIVE_VISUAL_ID, &nativeVid))
    {
        printf("Could not get native visual id\n");
        CloseNativeDisplay(nativeDisplay);
        return GL_FALSE;
    }

    EGLNativeWindowType nativeWin;
    if(!CreateNativeWin(nativeDisplay, 640, 480, nativeVid, &nativeWin))
    {
        printf("Could not create window\n");
        CloseNativeDisplay(nativeDisplay);
        return GL_FALSE;
    }

    // Create a surface for the main window
    EGLSurface eglSurface;
    eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, nativeWin, NULL);
    if (eglSurface == EGL_NO_SURFACE)
    {
        printf("Could not create EGL surface\n");
        DestroyNativeWin(nativeDisplay, nativeWin);
        CloseNativeDisplay(nativeDisplay);
        return GL_FALSE;
    }
    ctx.eglSurface = eglSurface;

    // Create an OpenGL ES context
    EGLContext eglContext;
    eglContext = eglCreateContext(eglDisplay, eglConfig, EGL_NO_CONTEXT, NULL);
    if (eglContext == EGL_NO_CONTEXT)
    {
        printf("Could not create EGL context\n");
        DestroyNativeWin(nativeDisplay, nativeWin);
        CloseNativeDisplay(nativeDisplay);
        return GL_FALSE;
    }

    // Make the context and surface current
    bsuccess = eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);
    if(!bsuccess)
    {
        printf("Could not activate EGL context\n");
        DestroyNativeWin(nativeDisplay, nativeWin);
        CloseNativeDisplay(nativeDisplay);
        return GL_FALSE;
    }

    ctx.nativeDisplay = nativeDisplay;
    ctx.nativeWin = nativeWin;
    return GL_TRUE;
}

GLboolean CreateProgram(esContext &ctx)
{
    const GLchar* vsSource =
      "uniform mat4 mvpMatrix;"
      "attribute vec4 vertPosition;"
      "attribute vec3 normal;"
      "attribute vec2 texCoord0;"
      "varying vec2 vTexCoord;"
      "varying vec3 vNormal;"
      "void main()"
      "{"
      "   gl_Position = mvpMatrix * vertPosition;"
      "   vTexCoord   = texCoord0;"
      "   vNormal     = normal;"
      "}";
    const GLchar* fsSource =
      "uniform vec4 lightVec;"
      "uniform sampler2D textureUnit0;"
      "varying vec2 vTexCoord;"
      "varying vec3 vNormal;"
      "void main()"
      "{"
      "  vec4 diff = vec4(dot(lightVec.xyz,normalize(vNormal)).xxx, 1);"
      "  gl_FragColor =  diff * texture(textureUnit0, vTexCoord);"
      "}";
    GLint status;

    // create and compile the vertex shader
    GLuint vs;
    vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vsSource, NULL);
    glCompileShader(vs);
    glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
    if (status == 0)
    {
        printf("Failed to create a vertex shader.\n");
        glDeleteShader(vs);
        return GL_FALSE;
    }

    // create and compile the fragment shader
    GLuint fs;
    fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fsSource, NULL);
    glCompileShader(fs);
    glGetShaderiv(fs, GL_COMPILE_STATUS, &status);
    if (status == 0)
    {
        printf("Failed to create a fragment shader.\n");
        glDeleteShader(vs);
        glDeleteShader(fs);
        return GL_FALSE;
    }

    // create the program and attach the shaders
    GLuint po;
    po = glCreateProgram();
    if (po == 0)
    {
        printf("Failed to create a program.\n");
        return GL_FALSE;
    }
    glAttachShader(po, vs);
    glAttachShader(po, fs);

    // link the program
    glLinkProgram(po);
    glGetProgramiv(po, GL_LINK_STATUS, &status);
    if(!status) 
    {
        printf("Failed to link program.\n");
        glDeleteProgram(po);
        glDeleteShader(vs);
        glDeleteShader(fs);
        return GL_FALSE;
    }

    // Free up no longer needed shader resources
    glDeleteShader(vs);
    glDeleteShader(fs);

    ctx.rs.po = po;
    ctx.rs.vertLoc     = glGetAttribLocation( ctx.rs.po, "vertPosition" );
    ctx.rs.normalLoc   = glGetAttribLocation( ctx.rs.po, "normal" );
    ctx.rs.texcoordLoc = glGetAttribLocation( ctx.rs.po, "texCoord0" );
    ctx.rs.mvpLoc      = glGetUniformLocation( ctx.rs.po, "mvpMatrix" );
    ctx.rs.lightLoc    = glGetUniformLocation( ctx.rs.po, "lightVec" );
    ctx.rs.texUnitLoc  = glGetUniformLocation( ctx.rs.po, "textureUnit0" );
    assert(ctx.rs.vertLoc >= 0);
    assert(ctx.rs.normalLoc >= 0);
    assert(ctx.rs.texcoordLoc >= 0);
    assert(ctx.rs.mvpLoc >= 0);
    assert(ctx.rs.lightLoc >= 0);
    assert(ctx.rs.texUnitLoc >= 0);

    return GL_TRUE;
}

void Render(esContext &ctx)
{
    // get program object
    GLuint po = ctx.rs.po;
    // get model properties
    GLuint texture = ctx.rs.ninjaTex[0];
    SBObject* ninja = &ctx.rs.ninja;
    GLuint arraystart = ninja->GetFirstFrameVertex(0);
    GLuint arraycount = ninja->GetFrameVertexCount(0);

    // get vertex data
    GLint posAttrib = ctx.rs.vertLoc;
    GLint normAttrib = ctx.rs.normalLoc;
    GLint uvAttrib = ctx.rs.texcoordLoc;
    int posSize = ninja->GetAttribComponents(0);
    int normSize = ninja->GetAttribComponents(1);
    int uvSize = ninja->GetAttribComponents(2);
    unsigned char * data_pointer = ninja->GetVertexData();
    GLuint numverts = ninja->GetNumVertices();
    void* posPtr = data_pointer;
    data_pointer += posSize * sizeof(GLfloat) * numverts;
    void* normPtr = data_pointer;
    data_pointer += normSize * sizeof(GLfloat) * numverts;
    void* uvPtr = data_pointer;

    // calculate the view matrix from the pitch and yaw of mouse movements
    vec4 target = vec4(0,85,0,0);
    mat4 yawmtx(mat4::rotate(ctx.rs.yaw, vec4(0,1,0,0)));
    mat4 pitchmtx(mat4::rotate(ctx.rs.pitch, vec4(1,0,0,0)));
    vec4 eye = target + (yawmtx * pitchmtx * vec4(0,0,200,1));
    mat4 view(mat4::lookAt(eye, target,vec4(0,1,0,0)));
    // calculate the projection matrix
    mat4 proj(mat4::perspective(60, ((float) ctx.nWindowWidth)/ctx.nWindowHeight, 1, 1000));
    // calvulate the view-projection matrix
    mat4 mvp = proj * view;
    // the light vector is the normalized direction vector pointing
    // from the eye to the origin.
    vec4 light = vec4::normalize(vec4(eye.x, eye.y, eye.z, 0));

    glClearColor ( 0.7f, 0.7f, 0.7f, 0.0f );
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // bind the program
    glUseProgram(po);
    glUniform4fv(ctx.rs.lightLoc, 1, &light.x);
    glUniformMatrix4fv(ctx.rs.mvpLoc, 1, GL_FALSE, &mvp.x.x);
    // the sampler should use texture unit 0
    glUniform1i(ctx.rs.texUnitLoc, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    // set state
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    // set vertex pointers
    glVertexAttribPointer(posAttrib, posSize, GL_FLOAT, GL_FALSE, 0, posPtr);
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(normAttrib, normSize, GL_FLOAT, GL_FALSE, 0, normPtr);
    glEnableVertexAttribArray(normAttrib);
    glVertexAttribPointer(uvAttrib, uvSize, GL_FLOAT, GL_FALSE, 0, uvPtr);
    glEnableVertexAttribArray(uvAttrib);
    // draw
    glDrawArrays(GL_TRIANGLES, arraystart, arraycount);
    // clean up state
    glDisable(GL_DEPTH_TEST);
    glUseProgram(0);
    // flip the visible buffer
    eglSwapBuffers(ctx.eglDisplay, ctx.eglSurface);
}

int main(int argc, char** argv)
{
    ctx.nWindowWidth  = 640;
    ctx.nWindowHeight = 480;
    int lRet = 0;

    // create window and setup egl
    if(Setup(ctx) == GL_FALSE)
    {
        return lRet;
    }

    // create the GLSL program
    if (!CreateProgram(ctx))
    {
        printf("Failed to Setup state.\n");
        return lRet;
    }

    // load the model
    if (!ctx.rs.ninja.LoadFromSBM("./ninja/ninja.sbm"))
    {
        printf("Failed load the model.\n");
        return lRet;
    }

    // load the texture
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, ctx.rs.ninjaTex);
    glBindTexture(GL_TEXTURE_2D, ctx.rs.ninjaTex[0]);
    if (!LoadTexture(ctx))
    {
        printf("Failed load the texture.\n");
        return lRet;
    }

    // main loop
    while (UpdateNativeWin(ctx.nativeDisplay, ctx.nativeWin))
    {
        // render the model
        Render(ctx);
    }

    eglMakeCurrent(EGL_NO_DISPLAY, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(ctx.eglDisplay, ctx.eglContext);
    eglDestroySurface(ctx.eglDisplay, ctx.eglSurface);
    eglTerminate(ctx.eglDisplay);
    DestroyNativeWin(ctx.nativeDisplay, ctx.nativeWin);
    CloseNativeDisplay(ctx.nativeDisplay);

    return lRet;
}
