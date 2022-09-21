#include <GL/glew.h>
#include <glm/glm.hpp>

class PGraph
{
    GLuint programObject;
    GLint  pointSizeLocation;
    GLint  projectionLocation;
    GLint  pointTextureLocation;
    GLuint positionVBO;
    GLuint colorVBO;
    GLuint VAO;
    GLuint pointTexture;
    int    Npoints_max;
    float  pointSize;
    float  drawBounds;
public:
    PGraph(int Npoints_max, float pointSize);
    ~PGraph();
    void SetColors(glm::vec4 *srcColors, int Npoints);
    void SetDrawBounds(float drawBounds);
    void Draw(glm::vec2* srcPoints, int Npoints);
};

