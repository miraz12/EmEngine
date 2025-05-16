#include "DebugDrawer.hpp"

DebugDrawer::DebugDrawer()
{
  lines.reserve(1000);
}

#ifdef EMSCRIPTEN
void
DebugDrawer::drawLine(const btVector3& from,
                      const btVector3& to,
                      const btVector3& color)
{
  // No implementation for Emscripten
}
#else
void
DebugDrawer::drawLine(const btVector3& from,
                      const btVector3& to,
                      const btVector3& color)
{
  // Only add the line if it's valid and we haven't exceeded the maximum
  if (!std::isnan(from.x()) && !std::isnan(from.y()) && !std::isnan(from.z()) &&
      !std::isnan(to.x()) && !std::isnan(to.y()) && !std::isnan(to.z())) {

    // No logging needed

    DebugDrawer::Line l;
    l.from = from;
    l.to = to;
    l.color = color;
    lines.push_back(l);
  }
}
#endif

void
DebugDrawer::drawContactPoint(const btVector3& /* pointOnB */,
                              const btVector3& /* normalOnB */,
                              btScalar /* distance */,
                              i32 /* lifeTime */,
                              const btVector3& /* color */)
{
  // Implement if needed
}

void
DebugDrawer::reportErrorWarning(const char* /* warningString */)
{
  // Implement if needed
}

void
DebugDrawer::draw3dText(const btVector3& /* location */,
                        const char* /* textString */)
{
  // Implement if needed
}

void
DebugDrawer::renderAndFlush(const glm::mat4& viewMatrix,
                            const glm::mat4& projMatrix,
                            ShaderProgram* externalShader)
{
#ifndef EMSCRIPTEN
  const size_t numLines = lines.size();

  if (numLines > 0) {
    // Process debug lines

    // Render only physics debug lines in 3D mode
    glEnable(GL_DEPTH_TEST);

    // Save the current program to restore it later
    GLint previousProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &previousProgram);

    // Set up the shader program
    if (externalShader) {
      // Use the external shader provided by DebugPass
      externalShader->use();

      // Set the model matrix to identity
      glm::mat4 identity(1.0f);

      // Set the matrices in the external shader
      glUniformMatrix4fv(externalShader->getUniformLocation("modelMatrix"),
                         1,
                         GL_FALSE,
                         glm::value_ptr(identity));
      glUniformMatrix4fv(externalShader->getUniformLocation("viewMatrix"),
                         1,
                         GL_FALSE,
                         glm::value_ptr(viewMatrix));
      glUniformMatrix4fv(externalShader->getUniformLocation("projMatrix"),
                         1,
                         GL_FALSE,
                         glm::value_ptr(projMatrix));
    } else {
      // Fallback to default OpenGL rendering if no shader is provided
      // (this should not normally happen)

      // Most basic rendering setup
      glMatrixMode(GL_PROJECTION);
      glLoadMatrixf(glm::value_ptr(projMatrix));
      glMatrixMode(GL_MODELVIEW);
      glLoadMatrixf(glm::value_ptr(viewMatrix));
    }

    // Create arrays for our vertex data
    std::vector<float> vertexData;
    vertexData.reserve(numLines * 2 * 6);

    // Fill the buffers with data from all lines
    for (size_t i = 0; i < numLines; i++) {
      const Line& line = lines[i];

      // Skip any invalid lines
      if (std::isnan(line.from.x()) || std::isnan(line.to.x())) {
        continue;
      }

      // First vertex (from)
      vertexData.push_back(line.from.x());
      vertexData.push_back(line.from.y());
      vertexData.push_back(line.from.z());
      vertexData.push_back(line.color.x());
      vertexData.push_back(line.color.y());
      vertexData.push_back(line.color.z());

      // Second vertex (to)
      vertexData.push_back(line.to.x());
      vertexData.push_back(line.to.y());
      vertexData.push_back(line.to.z());
      vertexData.push_back(line.color.x());
      vertexData.push_back(line.color.y());
      vertexData.push_back(line.color.z());
    }

    // Skip rendering if we have no valid vertex data beyond the overlay
    if (!vertexData.empty()) {
      // Render the collected lines

      // For maximum visibility, we disable depth testing first
      glDisable(GL_DEPTH_TEST);

      // Make sure we're drawing to the default framebuffer (screen)
      glBindFramebuffer(GL_FRAMEBUFFER, 0);

      // Ensure blending is enabled for transparency
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      // Create and set up buffers
      GLuint VAO, VBO;
      glGenVertexArrays(1, &VAO);
      glGenBuffers(1, &VBO);

      // Bind the VAO
      glBindVertexArray(VAO);

      // Upload interleaved vertex/color data
      glBindBuffer(GL_ARRAY_BUFFER, VBO);
      glBufferData(GL_ARRAY_BUFFER,
                   vertexData.size() * sizeof(float),
                   vertexData.data(),
                   GL_STATIC_DRAW);

      // Set up attribute pointers for the interleaved data
      // Position attribute (3 floats)
      glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
      glEnableVertexAttribArray(0);

      // Color attribute (3 floats, offset after position)
      glVertexAttribPointer(1,
                            3,
                            GL_FLOAT,
                            GL_FALSE,
                            6 * sizeof(float),
                            (void*)(3 * sizeof(float)));
      glEnableVertexAttribArray(1);

      // Increase line width for better visibility
      glLineWidth(7.0f);

      // Draw all lines in a single call
      glDrawArrays(GL_LINES, 0, vertexData.size() / 6);

      // Clean up
      glDeleteBuffers(1, &VBO);
      glDeleteVertexArrays(1, &VAO);

      // Restore previous shader program
      glUseProgram(previousProgram);
    }

    // Clear the lines vector
    lines.clear();
  }
#endif
}
