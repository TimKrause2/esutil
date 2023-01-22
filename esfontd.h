#include <GL/glew.h>
#include <glm/glm.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "esfontbase.h"

struct CharacterD
{
    GLuint    alphaTexture;
    glm::vec3 scale; // scaling factor for the quad
    glm::vec3 v0;    // location of v0 relative to baseline
    float     advance; // amount to advance on the baseline
};

class FreeTypeFontDynamic
{
private:
    bool      loaded;
    bool      quad_initialized;
    CharacterD characters[128];
    GLuint    program;
    GLuint    vbo; // vertex buffer object
    GLuint    vao; // vertex array object
    GLint     mvp_loc;
    GLint     alphaTexture_loc;
    GLuint    Colors_ubo; // uniform buffer object
    GLuint    Colors_loc;
    float     descender;

    void InitQuad(void);
    void DeleteQuad(void);

    void LoadCharacter(
            FT_Library library,
            FT_Face face,
            unsigned char charcode,
            float outlineWidth );

public:
    FreeTypeFontDynamic( void );
    ~FreeTypeFontDynamic( );

	void LoadOutline( const char *filename, unsigned int fontsize,
                      float outlineWidth);
    void Free( void );
    void Printf( double x, double y,
                 glm::vec4 &baseColor, glm::vec4 &outlineColor,
                 const char *format, ... );
    float PrintfAdvance(const char *format, ...);
};
