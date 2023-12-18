#include <GL/glew.h>
#include <glm/glm.hpp>

class TexturedQuad
{
private:
    bool      quad_initialized;
    int       width;
    int       height;
    GLuint    texture;
    GLuint    program;
    GLuint    vbo;
    GLuint    vao;
    GLint     mvp_loc;
    GLint     s_texture_loc;

    void InitQuad(void);
    void DeleteQuad(void);

public:
    TexturedQuad(int width, int height);
    ~TexturedQuad( );

    void LoadTexture(char *data);
    void RenderQuad(glm::mat4 &projection,
                    glm::mat4 &camera,
                    glm::mat4 &model,
                    glm::vec3 &scale);
};

struct Attributes
{
    glm::vec2 vertex;
    glm::vec2 texel;
};
