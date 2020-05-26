//
// Created by light on 2020/5/6.
//
#include <fstream>
#include <iostream>
#include <vector>
#include <filesystem>

#include <glad/glad.h>
#include <stb_image.h>
#include <learnopengl/model.h>

using namespace std;
namespace fs = std::experimental::filesystem;

unsigned int TextureFromFile(const char *path, const string &directory, bool gamma) {
    string normalized_path(path);
    normalized_path.erase(std::unique(normalized_path.begin(), normalized_path.end(), [](char a, char b) {
        return a == '\\' && b == '\\';
    }), normalized_path.end());
    string filename = (fs::path(directory) / fs::path(normalized_path)).generic_string();

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        stbi_image_free(data);
    } else {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}
