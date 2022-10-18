#pragma once
#include "System.hpp"
#include "../../Camera.hpp"
#include "../../ShaderPrograms/SimpleShaderProgram.hpp"

#include <array>


class GraphicsSystem : public System
{
public:

	GraphicsSystem(ECSManager *ECSManager);
	void update(float dt);

	Camera* getCamera() { return &m_camera; };

private:
	void draw();
    void initGL();

    Camera m_camera;
    SimpleShaderProgram m_simpleShaderProgram;

    // Framebuffer variables
    std::array<unsigned int, 2> m_fbos;
    std::array<unsigned int, 2> m_colTexs;
    std::array<unsigned int, 2> m_rbos;

    unsigned int m_width, m_height;
    unsigned int quadVAO, quadVBO;
};
