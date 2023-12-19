#include "txquad.h"
#include "esShader.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

TexturedQuad::TexturedQuad(int width, int height)
    : width(width),
    height(height),
    quad_initialized(false)
{
    InitQuad();
}

TexturedQuad::~TexturedQuad(void)
{
    DeleteQuad();
}

#define VERTEX_LOC 0
#define TEXEL_LOC  1

void TexturedQuad::InitQuad(void)
{
    const char *vShaderSrc =
        "#version 300 es\n"
        "layout(location = 0) in vec2 a_vertex;\n"
        "layout(location = 1) in vec2 a_tex;\n"
        "uniform mat4 mvp;\n"
        "out vec2 tex;\n"
        "void main(void)\n"
        "{\n"
        "   gl_Position = mvp*vec4(a_vertex.xy,0.0,1.0);\n"
        "   tex = a_tex;\n"
        "}\n";

    const char *fShaderSrc =
        "#version 300 es\n"
        "precision highp float;\n"
        "in vec2 tex;\n"
        "layout(location =0) out vec4 outColor;\n"
        "uniform sampler2D s_texture;\n"
        "void main(void)\n"
        "{\n"
        "   outColor = texture(s_texture, tex);\n"
        "}\n";

    program = esLoadProgram(vShaderSrc, fShaderSrc);
    if(!program){
        return;
    }

    mvp_loc = glGetUniformLocation(program, "mvp");
    s_texture_loc = glGetUniformLocation(program, "s_texture");

    Attributes attributes[4] =
        {
            {glm::vec2(-0.5,-0.5),glm::vec2(0.0,1.0)},
            {glm::vec2( 0.5,-0.5),glm::vec2(1.0,1.0)},
            {glm::vec2(-0.5, 0.5),glm::vec2(0.0,0.0)},
            {glm::vec2( 0.5, 0.5),glm::vec2(1.0,0.0)}
        };

    glGenBuffers(1, &vbo);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(attributes),
                 attributes, GL_STATIC_DRAW);
    glVertexAttribPointer (VERTEX_LOC, 2, GL_FLOAT, GL_FALSE, sizeof(Attributes), (void*)offsetof(Attributes,vertex));
    glVertexAttribPointer (TEXEL_LOC, 2, GL_FLOAT, GL_FALSE, sizeof(Attributes), (void*)offsetof(Attributes,texel));
    glEnableVertexAttribArray(VERTEX_LOC);
    glEnableVertexAttribArray(TEXEL_LOC);

    glBindVertexArray(0);

    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexStorage2D(GL_TEXTURE_2D, 1,
                   GL_RGB8, width, height);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE );

    quad_initialized = true;
}

void TexturedQuad::DeleteQuad(void)
{
    if(!quad_initialized)return;
    glDeleteProgram(program);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteTextures(1, &texture);
    quad_initialized = false;
}

void TexturedQuad::LoadTexture(char *data)
{
    if(!quad_initialized)return;
    glBindTexture( GL_TEXTURE_2D, texture);
    glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, width, height,
                    GL_RGB, GL_UNSIGNED_BYTE, data);
}

void TexturedQuad::RenderQuad(glm::mat4 &Mprojection,
                              glm::mat4 &Mcamera,
                              glm::mat4 &Mmodel,
                              glm::vec3 &scale)
{
    if(!quad_initialized)return;
    glUseProgram(program);
    glBindVertexArray(vao);
    glUniform1i(s_texture_loc, 0);
    glActiveTexture(GL_TEXTURE0);
    glm::mat4 Mscale = glm::scale(scale);
    glm::mat4 Mcam_inv = glm::inverse(Mcamera);
    glm::mat4 mvp = Mprojection*Mcam_inv*Mmodel*Mscale;
    glUniformMatrix4fv(mvp_loc, 1, GL_FALSE, glm::value_ptr(mvp));
    glBindTexture(GL_TEXTURE_2D, texture);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glUseProgram(0);
    glBindVertexArray(0);
}
