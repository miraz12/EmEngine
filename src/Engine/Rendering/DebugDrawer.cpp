#include "DebugDrawer.hpp"
DebugDrawer::DebugDrawer() { lines.reserve(1000); }

#ifdef EMSCRIPTEN
void DebugDrawer::drawLine(const btVector3 & /* from */, const btVector3 & /* to */,
                           const btVector3 & /* color */) {
#else
void DebugDrawer::drawLine(const btVector3 &from, const btVector3 &to,
                           const btVector3 & /* color */) {
  DebugDrawer::Line l;
  l.from = from;
  l.to = to;
  lines.push_back(l);

#endif
}

void DebugDrawer::drawContactPoint(const btVector3 & /* pointOnB */,
                                   const btVector3 & /* normalOnB */, btScalar /* distance */,
                                   int /* lifeTime */, const btVector3 & /* color */) {
  // Implement if needed
}

void DebugDrawer::reportErrorWarning(const char * /* warningString */) {
  // Implement if needed
}

void DebugDrawer::draw3dText(const btVector3 & /* location */, const char * /* textString */) {
  // Implement if needed
}

void DebugDrawer::renderAndFlush() {
#ifndef EMSCRIPTEN
  if (lines.size() > 0) {

    // load the vertex data info
    glVertexAttribPointer(
        0,                 // the handle for the a_position shader attrib
        3,                 // there are 3 values xyz
        GL_FLOAT,          // they a float
        GL_FALSE,          // don't need to be normalised
        4 * sizeof(float), // how many floats to the next one(be aware btVector3 uses 4 floats)
        (GLfloat *)&this->lines[0] // where do they start as an index); // use 3 values, but add
                                   // stride each time to get to the next
    );

    glEnableVertexAttribArray(0);
    glDrawArrays(GL_LINES, 0, lines.size() * 2);

    lines.clear();
  }
#endif
}
