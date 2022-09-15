#include <GL/glew.h>
#include <glm/glm.hpp>

#define X_LOC 0
#define Y_LOC 1

class LGraph
{
    static int    ref_count;
    static GLuint programObject;
    static GLint  colorLocation;
    static GLint  projectionLocation;
    GLuint xVBO;
    GLuint yVBO;
    GLuint VAO;
    int Nvertices;
    glm::vec4 color0;
    glm::vec4 color1;
    float lineWidth0;
    float lineWidth1;
    float ytop;
    float ybottom;

    void ProgramLoad(void);
    void ProgramDestroy(void);

public:
    LGraph(int Nvertices);
    ~LGraph(void);
    void SetColors(glm::vec4 &color0, glm::vec4 &color1);
    void SetLineWidths(float lineWidth0, float lineWidth1);
    void SetLimits(float ytop, float ybottom);
    void Draw(float *y);
};
