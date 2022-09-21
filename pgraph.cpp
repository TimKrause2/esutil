#include "pgraph.h"
#include "esShader.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdio.h>
#include <string.h>

#define POSITION_LOC 0
#define COLOR_LOC 1

PGraph::PGraph(int Npoints_max, float pointSize)
    :Npoints_max(Npoints_max),
      pointSize(pointSize),
      drawBounds(1.0)
{
    const char *vertShaderSrc =
            "#version 300 es\n"
            "layout(location=0) in vec2 position;\n"
            "layout(location=1) in vec4 color_in;\n"
            "uniform mat4 projection;\n"
            "uniform float pointSize;\n"
            "out vec4 pointColor;\n"
            "void main(void)\n"
            "{\n"
            "   gl_Position = projection*vec4(position,0.0,1.0);\n"
            "   pointColor = color_in;\n"
            "   gl_PointSize = pointSize;\n"
            "}\n";

    const char *fragShaderSrc =
            "#version 300 es\n"
            "precision highp float;\n"
            "in vec4 pointColor;\n"
            "uniform sampler2D pointTexture;\n"
            "layout(location=0) out vec4 fragColor;\n"
            "void main()\n"
            "{\n"
            "   vec4 texel = texture(pointTexture, gl_PointCoord);\n"
            "   fragColor = vec4(pointColor.rgb, pointColor.a*texel.r);\n"
            "}\n";

    programObject = esLoadProgram(vertShaderSrc, fragShaderSrc);
    if(!programObject){
        printf("pgraph. Couldn't load program.\n");
        return;
    }

    projectionLocation = glGetUniformLocation(programObject, "projection");
    pointSizeLocation = glGetUniformLocation(programObject, "pointSize");
    pointTextureLocation = glGetUniformLocation(programObject, "pointTexture");

    glGenBuffers(1, &positionVBO);
    glGenBuffers(1, &colorVBO);

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2)*Npoints_max,
                 NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(POSITION_LOC, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(POSITION_LOC);

    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4)*Npoints_max,
                 NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(COLOR_LOC, 4, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(COLOR_LOC);

    glBindVertexArray(0);

    pointSize = ceil(pointSize);
    int textureSize = (int)pointSize;

    unsigned char *image = new unsigned char[textureSize*textureSize];
    unsigned char *pixel = image;
    float pixelsize = 2.0/(textureSize-1);
    for(int y=0;y<textureSize;y++){
        float dy = (float)y/(textureSize-1);
        dy*=2.0;
        dy-=1.0;
        for(int x=0;x<textureSize;x++){
            float dx = (float)x/(textureSize-1);
            dx*=2.0;
            dx-=1.0;
            float r=sqrt(dx*dx + dy*dy);
            if(r<=1.0){
                *pixel = 255;
            }else if(r<=(1.0+pixelsize)){
                float a = 1.0 - (r-1.0)/pixelsize;
                *pixel = a*255;
            }else{
                *pixel = 0;
            }
            pixel++;
        }
    }

    glGenTextures(1, &pointTexture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, pointTexture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R8, textureSize, textureSize);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textureSize, textureSize,
                    GL_R, GL_UNSIGNED_BYTE, image);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

}

void PGraph::SetColors(glm::vec4 *srcColors, int Npoints)
{
    if(!programObject)return;
    if(Npoints>Npoints_max)Npoints = Npoints_max;

    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    glm::vec4 *map = (glm::vec4*)glMapBufferRange(GL_ARRAY_BUFFER,
                                                  0, sizeof(glm::vec4)*Npoints,
                                                  GL_MAP_WRITE_BIT|
                                                  GL_MAP_INVALIDATE_BUFFER_BIT);
    memcpy(map, srcColors, sizeof(glm::vec4)*Npoints);

    glUnmapBuffer(GL_ARRAY_BUFFER);
}

void PGraph::SetDrawBounds(float drawBounds)
{
    PGraph::drawBounds = drawBounds;
}

void PGraph::Draw(glm::vec2 *srcPoints, int Npoints)
{
    if(!programObject)return;
    if(Npoints>Npoints_max)Npoints=Npoints_max;

    glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
    glm::vec2 *map = (glm::vec2*)glMapBufferRange(GL_ARRAY_BUFFER,
                                                  0, sizeof(glm::vec2)*Npoints,
                                                  GL_MAP_WRITE_BIT|
                                                  GL_MAP_INVALIDATE_BUFFER_BIT);
    memcpy(map, srcPoints, sizeof(glm::vec2)*Npoints);

    glUnmapBuffer(GL_ARRAY_BUFFER);

    glUseProgram(programObject);

    GLint	viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    float left;
    float right;
    float top;
    float bottom;
    float nearVal = 1.0;
    float farVal = -1.0;
    int width = viewport[2];
    int height = viewport[3];
    if(width>height){
        float aspect = (float)width/height;
        top = drawBounds;
        bottom = -drawBounds;
        left = -drawBounds*aspect;
        right = drawBounds*aspect;
    }else{
        float aspect = (float)height/width;
        top = drawBounds*aspect;
        bottom = -drawBounds*aspect;
        left = -drawBounds;
        right = drawBounds;
    }

    glm::mat4 projection = glm::ortho(left, right, bottom, top, nearVal, farVal);

    glUniformMatrix4fv(projectionLocation,
                       1, GL_FALSE, glm::value_ptr(projection));
    glUniform1f(pointSizeLocation, pointSize);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, pointTexture);
    glUniform1i(pointTextureLocation, 0);

    glBindVertexArray(VAO);

    glDrawArrays(GL_POINTS, 0, Npoints);

    glBindVertexArray(0);
    glUseProgram(0);
}
