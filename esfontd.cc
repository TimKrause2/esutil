﻿#include "esfontd.h"
#include "esShader.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

FreeTypeFontDynamic::FreeTypeFontDynamic( void ){
    loaded = false;
    quad_initialized = false;
}

FreeTypeFontDynamic::~FreeTypeFontDynamic( void ){
	Free();
    DeleteQuad();
}

void FreeTypeFontDynamic::Free( void )
{
    if(!loaded)return;
    for(int c=1;c<128;c++){
        if(characters[c].alphaTexture){
            glDeleteTextures(1, &characters[c].alphaTexture);
        }
    }
    loaded = false;
}

#define VERTEX_LOC 0
#define TEXEL_LOC  1
#define COLORS_BINDING_POINT 1

struct Colors
{
    glm::vec4 baseColor;
    glm::vec4 outlColor;
};

void FreeTypeFontDynamic::InitQuad(void)
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
            "precision mediump float;\n"
            "in vec2 tex;\n"
            "layout(location =0) out vec4 outColor;\n"
            "uniform sampler2D alphaTexture;\n"
            "layout(std140) uniform Colors\n"
            "{\n"
            "   vec4 baseColor;\n"
            "   vec4 outlColor;\n"
            "};\n"
            "void main(void)\n"
            "{\n"
            "   vec4 alpha = texture(alphaTexture, tex);\n"
            "   if(alpha.g==0.0) discard;\n"
            "   float baseAlpha = baseColor.a*alpha.r;\n"
            "   float outlAlpha = outlColor.a*alpha.g;\n"
            "   outColor = vec4(outlColor.rgb*(1.0-baseAlpha)*outlAlpha +\n"
            "                   baseColor.rgb*baseAlpha,\n"
            "                   max(baseAlpha, outlAlpha));\n"
            "}\n";

    program = esLoadProgram(vShaderSrc, fShaderSrc);
    if(!program){
        printf("esfontd.cc couldn't load program.\n");
        return;
    }

    mvp_loc = glGetUniformLocation(program, "mvp");
    alphaTexture_loc = glGetUniformLocation(program, "alphaTexture");
    Colors_loc = glGetUniformBlockIndex(program, "Colors");
    glUniformBlockBinding(program, Colors_loc, COLORS_BINDING_POINT);

    Attributes attributes[4] =
    {
        {glm::vec2(0.0,0.0),glm::vec2(0.0,1.0)},
        {glm::vec2(1.0,0.0),glm::vec2(1.0,1.0)},
        {glm::vec2(0.0,1.0),glm::vec2(0.0,0.0)},
        {glm::vec2(1.0,1.0),glm::vec2(1.0,0.0)}
    };

    glGenBuffers(1, &vbo);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(attributes),
                 attributes, GL_STATIC_DRAW);
    glVertexAttribPointer(VERTEX_LOC, 2, GL_FLOAT, GL_FALSE, sizeof(Attributes), (void*)offsetof(Attributes,vertex));
    glVertexAttribPointer(TEXEL_LOC, 2, GL_FLOAT, GL_FALSE, sizeof(Attributes), (void*)offsetof(Attributes,texel));
    glEnableVertexAttribArray(VERTEX_LOC);
    glEnableVertexAttribArray(TEXEL_LOC);

    glBindVertexArray(0);

    glGenBuffers(1, &Colors_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, Colors_ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(Colors), NULL, GL_DYNAMIC_DRAW);

    quad_initialized = true;
}

void FreeTypeFontDynamic::DeleteQuad(void)
{
    if(!quad_initialized)return;
    glDeleteProgram(program);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &Colors_ubo);
    glDeleteVertexArrays(1, &vao);
    quad_initialized = false;
}

#define TEXT_BUF_LENGTH 1024
static char text[TEXT_BUF_LENGTH];

void FreeTypeFontDynamic::Printf(
        double x, double y,
        glm::vec4 &baseColor, glm::vec4 &outlColor,
        const char *format, ... )
{
    if(!loaded || !quad_initialized)return;
	va_list ap;
	va_start( ap, format );
	vsnprintf( text, TEXT_BUF_LENGTH, format, ap );
	va_end( ap );

	GLint	viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
    float left = 0.0;
    float right = viewport[2];
    float top = viewport[3];
    float bottom = 0.0;
    float nearVal = 1.0;
    float farVal = -1.0;
    glm::mat4 Mprojection = glm::ortho(left, right, bottom, top, nearVal, farVal);

    glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    glm::vec3 vBaseLine(x, y-descender, 0.0);

    glUseProgram(program);

    glBindBuffer(GL_UNIFORM_BUFFER, Colors_ubo);
    Colors *colors_map = (Colors*)glMapBufferRange(
                GL_UNIFORM_BUFFER,
                0, sizeof(Colors),
                GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

    colors_map->baseColor = baseColor;
    colors_map->outlColor = outlColor;

    glUnmapBuffer(GL_UNIFORM_BUFFER);

    glBindBufferBase(GL_UNIFORM_BUFFER, COLORS_BINDING_POINT, Colors_ubo);

    glUniform1i(alphaTexture_loc, 0);
    glActiveTexture(GL_TEXTURE0);

    glBindVertexArray(vao);

    for(char *c=text; *c; c++){
        if(characters[*c].alphaTexture){
            glm::mat4 Mscale = glm::scale(characters[*c].scale);
            glm::mat4 Mtrans = glm::translate(vBaseLine + characters[*c].v0);
            glm::mat4 mvp = Mprojection*Mtrans*Mscale;
            glUniformMatrix4fv(mvp_loc, 1, GL_FALSE, glm::value_ptr(mvp));

            glBindTexture(GL_TEXTURE_2D, characters[*c].alphaTexture);

            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        }
        vBaseLine += glm::vec3(characters[*c].advance, 0.0, 0.0);
    }

    glUseProgram(0);
    glBindVertexArray(0);
}

float FreeTypeFontDynamic::PrintfAdvance(const char *format, ... )
{
    if(!loaded)return 0.0;
    va_list ap;
    va_start( ap, format );
    vsnprintf( text, TEXT_BUF_LENGTH, format, ap );
    va_end( ap );

    float advance = 0.0;

    for(char *c=text; *c; c++){
        advance += characters[*c].advance;
    }
    return advance;
}

// Each time the renderer calls us back we just push another span entry on
// our list.

static void
RasterCallback(const int y,
               const int count,
               const FT_Span * const spans,
               void * const user) 
{
  Spans *sptr = (Spans *)user;
  for (int i = 0; i < count; ++i) 
    sptr->push_back(Span(spans[i].x, y, spans[i].len, spans[i].coverage));
}


// Set up the raster parameters and render the outline.

static void
RenderSpans(FT_Library &library,
            FT_Outline * const outline,
            Spans *spans) 
{
  FT_Raster_Params params;
  memset(&params, 0, sizeof(params));
  params.flags = FT_RASTER_FLAG_AA | FT_RASTER_FLAG_DIRECT;
  params.gray_spans = RasterCallback;
  params.user = spans;

  FT_Outline_Render(library, outline, &params);
}

struct rg_pixel
{
    unsigned char r;
    unsigned char g;
};

void FreeTypeFontDynamic::LoadCharacter(
         FT_Library library,
         FT_Face face,
         unsigned char charcode,
         float outlineWidth )
{
// 	printf("make_dlist_outline: charcode=%hhd\n",charcode);
	// Load the glyph we are looking for.
	FT_UInt gindex = FT_Get_Char_Index(face, charcode);
	if (FT_Load_Glyph(face, gindex, FT_LOAD_NO_BITMAP) == 0)
	{
		// Need an outline for this to work.
		if (face->glyph->format == FT_GLYPH_FORMAT_OUTLINE)
		{
			// Render the basic glyph to a span list.
			Spans spans;
			RenderSpans(library, &face->glyph->outline, &spans);

			// Next we need the spans for the outline.
			Spans outlineSpans;

			// Set up a stroker.
			FT_Stroker stroker;
			FT_Stroker_New(library, &stroker);
			FT_Stroker_Set(stroker,
                           (int)(outlineWidth * 64),
                           FT_STROKER_LINECAP_ROUND,
                           FT_STROKER_LINEJOIN_ROUND,
                           0);

			FT_Glyph glyph;
			if (FT_Get_Glyph(face->glyph, &glyph) == 0)
			{
				FT_Glyph_StrokeBorder(&glyph, stroker, 0, 1);
				// Again, this needs to be an outline to work.
				if (glyph->format == FT_GLYPH_FORMAT_OUTLINE)
				{
					// Render the outline spans to the span list
					FT_Outline *o =
						&reinterpret_cast<FT_OutlineGlyph>(glyph)->outline;
					RenderSpans(library, o, &outlineSpans);
				}

				// Clean up afterwards.
				FT_Stroker_Done(stroker);
				FT_Done_Glyph(glyph);

				// Now we need to put it all together.
				if (!spans.empty())
				{
					// Figure out what the bounding rect is for both the span lists.
					Rect rect(spans.front().x,
										spans.front().y,
										spans.front().x,
										spans.front().y);
					for (Spans::iterator s = spans.begin();
								s != spans.end(); ++s)
					{
						rect.Include(Vec2(s->x, s->y));
						rect.Include(Vec2(s->x + s->width - 1, s->y));
					}
					for (Spans::iterator s = outlineSpans.begin();
								s != outlineSpans.end(); ++s)
					{
						rect.Include(Vec2(s->x, s->y));
						rect.Include(Vec2(s->x + s->width - 1, s->y));
					}

					// This is unused in this test but you would need this to draw
					// more than one glyph.
// 					float bearingX = (float)face->glyph->metrics.horiBearingX/64;
// 					float bearingY = (float)face->glyph->metrics.horiBearingY/64;
// 					float advance = (float)face->glyph->advance.x/64;
// 					printf("bearingX:%f bearingY:%f advance:%f\n",bearingX,bearingY,advance);
// 					printf("rect: xmin:%f, xmax:%f, ymin:%f ymax:%f\n",
// 								 rect.xmin, rect.xmax, rect.ymin, rect.ymax );

					// Get some metrics of our image.
					int imgWidth = rect.Width(),
							imgHeight = rect.Height(),
							imgSize = imgWidth * imgHeight;

					// Allocate data for our image and clear it out to transparent.
                    rg_pixel *alpha_pxl = new rg_pixel[imgSize];
                    memset(alpha_pxl, 0, sizeof(rg_pixel)*imgSize);

					// Loop over the outline spans and just draw them into the
					// image.
					for (Spans::iterator s = outlineSpans.begin();
								s != outlineSpans.end(); ++s)
						for (int w = 0; w < s->width; ++w)
                            alpha_pxl[(int)((imgHeight - 1 - (s->y - rect.ymin)) * imgWidth
                                                + s->x - rect.xmin + w)].g =
                                s->coverage;

					for (Spans::iterator s = spans.begin();
								s != spans.end(); ++s)
						for (int w = 0; w < s->width; ++w)
                            alpha_pxl[(int)((imgHeight - 1 - (s->y - rect.ymin)) * imgWidth
                                                + s->x - rect.xmin + w)].r =
                                s->coverage;

					// write the image to a texture
                    glActiveTexture( GL_TEXTURE0 );
                    glGenTextures(1, &characters[charcode].alphaTexture);
                    glBindTexture( GL_TEXTURE_2D, characters[charcode].alphaTexture );
//					glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA,
//												imgWidth, imgHeight, 0,
//												GL_RGBA, GL_UNSIGNED_BYTE, pxl );
                    glTexStorage2D( GL_TEXTURE_2D, 1,
                                    GL_RG8, imgWidth, imgHeight);
                    glTexSubImage2D( GL_TEXTURE_2D, 0,
                                     0, 0,
                                     imgWidth, imgHeight,
                                     GL_RG, GL_UNSIGNED_BYTE, alpha_pxl );
                    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
					glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
					glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
					glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

                    characters[charcode].scale = glm::vec3(imgWidth,imgHeight,1.0);
                    characters[charcode].v0 = glm::vec3(rect.xmin, rect.ymin, 0.0);
                    characters[charcode].advance = (float)face->glyph->advance.x / 64;

                    descender = MIN(descender,rect.ymin);
                    delete [] alpha_pxl;
                }else{
                    characters[charcode].alphaTexture = 0;
                    characters[charcode].advance = (float)face->glyph->advance.x / 64;
				}
				
			}
		}
	}
}

void FreeTypeFontDynamic::LoadOutline(
        const char *filename, unsigned int fontsize,
        float outlineWidth)
{
    if(loaded)Free();
    if(!quad_initialized)InitQuad();
	FT_Library ftlibrary;
	FT_Face    ftface;

	FT_Error error = FT_Init_FreeType( &ftlibrary );
	if(error){
		printf("FT_Init_FreeType:0x%X\n",error);
		return;
	}

    error = FT_New_Face( ftlibrary, filename, 0, &ftface );
	if( error ){
		printf("FT_New_Face:0x%X\n",error);
		FT_Done_FreeType( ftlibrary );
		return;
	}
	
	if(!FT_IS_SCALABLE( ftface ) ){
		printf("Font is not scalable. Not using it.\n");
		FT_Done_Face( ftface );
		FT_Done_FreeType( ftlibrary );
		return;
	}

	error = FT_Set_Pixel_Sizes( ftface, fontsize, fontsize );
	if( error ){
		printf("FT_Set_Pixel_Sizes:0x%X\n",error);
		FT_Done_Face( ftface );
		FT_Done_FreeType( ftlibrary );
		return;
	}

    descender = 0.0;

	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    for( unsigned char c=1;c<128;c++)
        LoadCharacter( ftlibrary, ftface, c, outlineWidth );
	
	FT_Done_Face( ftface );
	FT_Done_FreeType( ftlibrary );

    loaded = true;
}



