//
// Created by light on 1/6/2020.
//

#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <learnopengl/shader_m.h>
#include <map>

struct Character {
    GLuint TextureID;  // 字形纹理的ID
    glm::ivec2 Size;       // 字形大小
    glm::ivec2 Bearing;    // 从基准线到字形左部/顶部的偏移值
    long Advance;    // 原点距下一个字形原点的距离
};

class FreeType {
public:
    FreeType() = delete;
    explicit FreeType(const char* filename);
    void renderText(std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color);
    void setMat4(const std::string &name, const glm::mat4 &mat) const;
    void use() const;

private:
    GLuint VAO{}, VBO{};
    Shader s = Shader("res/text.vs", "res/text.fs");
    std::map<uint8_t, Character> Characters;
};