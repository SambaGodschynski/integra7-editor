#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include "imgui.h"
#include <GLFW/glfw3.h>
#include <cstdio>

#ifndef GL_CLAMP_TO_EDGE
#  define GL_CLAMP_TO_EDGE 0x812F
#endif

ImTextureID LoadKnobTexture(const char* path)
{
    int w = 0, h = 0;
    unsigned char* data = stbi_load(path, &w, &h, NULL, 4);
    if (!data)
    {
        std::fprintf(stderr, "LoadKnobTexture: failed to load '%s'\n", path);
        return 0;
    }

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);

    return (ImTextureID)tex;
}
