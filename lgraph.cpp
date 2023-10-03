#include "lgraph.h"
#include "esShader.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string.h>
#include <stdio.h>

int    LGraph::ref_count = 0;
GLuint LGraph::programObject;
GLint  LGraph::colorLocation;
GLint  LGraph::projectionLocation;

void LGraph::ProgramLoad(void)
{
    if(!ref_count){
        const char *vertShaderSrc =
                "#version 300 es\n"
                "precision highp float;\n"
                "layout(location=0) in float a_x;\n"
                "layout(location=1) in float a_y;\n"
                "uniform mat4 projection;\n"
                "void main()\n"
                "{\n"
                "   gl_Position = projection*vec4(a_x,a_y,0.0,1.0);\n"
                "}\n";

        const char *fragShaderSrc =
                "#version 300 es\n"
                "precision mediump float;\n"
                "layout(location = 0) out vec4 f_color;\n"
                "uniform vec4 color;\n"
                "void main()\n"
                "{\n"
                "   f_color = color;\n"
                "}\n";

        programObject = esLoadProgram(vertShaderSrc, fragShaderSrc);
        if(!programObject){
            printf("lgraph.cpp: Error, couldn't load program.\n");
            return;
        }

        colorLocation = glGetUniformLocation(programObject, "color");
        projectionLocation = glGetUniformLocation(programObject, "projection");

    }
    ref_count++;
}

void LGraph::ProgramDestroy(void)
{
    if(ref_count){
        ref_count--;
        if(ref_count==0){
            glDeleteProgram(programObject);
        }
    }
}

LGraph::LGraph(int Nvertices)
    :Nvertices(Nvertices),
    lineWidth0(1.0),
    lineWidth1(3.0),
    ytop(1.0),
    ybottom(-1.0)
{
    ProgramLoad();

    glGenBuffers(1, &xVBO);
    glGenBuffers(1, &yVBO);

    glBindBuffer(GL_ARRAY_BUFFER, xVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*Nvertices,
                 NULL, GL_STATIC_DRAW);

    float *xVBOmap = (float*)glMapBufferRange(GL_ARRAY_BUFFER,
                                              0, sizeof(float)*Nvertices,
                                              GL_MAP_WRITE_BIT|
                                              GL_MAP_INVALIDATE_BUFFER_BIT);
    for(int i=0;i<Nvertices;i++){
        xVBOmap[i] = ((float)i/(Nvertices-1))*2.0 - 1.0;
    }

    glUnmapBuffer(GL_ARRAY_BUFFER);

    glBindBuffer(GL_ARRAY_BUFFER, yVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*Nvertices,
                 NULL, GL_STREAM_DRAW);

    glGenVertexArrays(1, &VAO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, xVBO);
    glVertexAttribPointer(X_LOC, 1, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(X_LOC);

    glBindBuffer(GL_ARRAY_BUFFER, yVBO);
    glVertexAttribPointer(Y_LOC, 1, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(Y_LOC);

    glBindVertexArray(0);

}

LGraph::~LGraph(void)
{
    ProgramDestroy();

    glDeleteBuffers(1, &xVBO);
    glDeleteBuffers(1, &yVBO);
    glDeleteVertexArrays(1, &VAO);
}

void LGraph::SetColors(glm::vec4 &color0, glm::vec4 &color1){
    LGraph::color0 = color0;
    LGraph::color1 = color1;
}

void LGraph::SetLineWidths(float lineWidth0, float lineWidth1)
{
    LGraph::lineWidth0 = lineWidth0;
    LGraph::lineWidth1 = lineWidth1;
}

void LGraph::SetLimits(float ytop, float ybottom)
{
    LGraph::ytop = ytop;
    LGraph::ybottom = ybottom;
}

void LGraph::SetX(float *x)
{
    glBindBuffer(GL_ARRAY_BUFFER, xVBO);

    float *xVBOmap = (float*)glMapBufferRange(GL_ARRAY_BUFFER,
                                              0, sizeof(float)*Nvertices,
                                              GL_MAP_WRITE_BIT|
                                              GL_MAP_INVALIDATE_BUFFER_BIT);
    for(int i=0;i<Nvertices;i++){
        xVBOmap[i] = x[i];
    }

    glUnmapBuffer(GL_ARRAY_BUFFER);
}


void LGraph::Draw(float *y0)
{
    if(ref_count==0)return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindBuffer(GL_ARRAY_BUFFER, yVBO);

    float *yVBOmap = (float*)glMapBufferRange(GL_ARRAY_BUFFER,
                                              0, sizeof(float)*Nvertices,
                                              GL_MAP_WRITE_BIT|
                                              GL_MAP_INVALIDATE_BUFFER_BIT);
    memcpy(yVBOmap, y0, sizeof(float)*Nvertices);

    glUnmapBuffer(GL_ARRAY_BUFFER);

    glUseProgram(programObject);

    glBindVertexArray(VAO);

    float top = ytop;
    float bottom = ybottom;
    float left = -1.0;
    float right = 1.0;
    float near =  1.0;
    float far = -1.0;
    glm::mat4 projection = glm::ortho(left, right, bottom, top, near, far);

    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));

    glUniform4fv(colorLocation, 1, glm::value_ptr(color0));
    glLineWidth(lineWidth0);
    glDrawArrays(GL_LINE_STRIP, 0, Nvertices);

    glUniform4fv(colorLocation, 1, glm::value_ptr(color1));
    glLineWidth(lineWidth1);
    glDrawArrays(GL_LINE_STRIP, 0, Nvertices);

    glBindVertexArray(0);
    glUseProgram(0);
}
