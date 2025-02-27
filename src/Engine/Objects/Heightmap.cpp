#include "Heightmap.hpp"

Heightmap::Heightmap(std::string filename)
  : m_filename(filename)
{
  i32 numChannels;
  i32 width;
  i32 height;
  stbi_set_flip_vertically_on_load(true); // Flip the image vertically
  unsigned char* imageData =
    stbi_load(filename.c_str(), &width, &height, &numChannels, 1);

  if (!imageData) {
    std::cout << "Failed to load HDR image." << std::endl;
  } else {

    std::vector<float> heights(width * height);
    for (i32 i = 0; i < width * height; ++i) {
      // Convert the pixel value to a normalized height value in the range [0,
      // 1]
      heights[i] = static_cast<float>(imageData[i]) / 255.0f;
    }

    float terrainScale = 10.1f; // Scale factor for the terrain

    // Generate vertices
    m_vertices.reserve(width * height);
    for (i32 z = 0; z < height; ++z) {
      for (i32 x = 0; x < width; ++x) {
        float heightValue = heights[z * width + x];
        float xPos = (x - std::ceil((width) / 2) + 0.5f);
        float yPos = (heightValue * terrainScale) - terrainScale * 0.5f;
        float zPos = (z - std::ceil((height) / 2) + 0.5f);
        m_vertices.push_back(glm::vec3(xPos, yPos, zPos));
        m_data.push_back(yPos);
      }
    }

    p_coll = new btHeightfieldTerrainShape(width,
                                           height,
                                           m_data.data(),
                                           1,
                                           -terrainScale,
                                           terrainScale,
                                           1,
                                           PHY_FLOAT,
                                           false);

    i32 numStrips = height - 1;
    i32 numDegens = 2 * (numStrips - 1);
    i32 verticesPerStrip = 2 * width;

    // Generate iindicesndices
    m_indices.reserve((verticesPerStrip * numStrips) + numDegens);
    for (i32 z = 0; z < height - 1; ++z) {
      if (z > 0) {
        m_indices.push_back(z * width);
      }

      for (i32 x = 0; x < width; ++x) {
        m_indices.push_back(z * width + x);
        m_indices.push_back((z + 1) * width + x);
      }

      if (z < height - 2) {
        m_indices.push_back((z + 1) * width + (width - 1));
      }
    }

    glm::mat4 modelMat = glm::identity<glm::mat4>();
    newNode(modelMat);

    p_numMeshes = 1;
    p_meshes = std::make_unique<Mesh[]>(p_numMeshes);
    p_meshes[0].numPrims = 1;
    p_meshes[0].m_primitives = std::make_unique<Primitive[]>(1);
    Primitive* newPrim = &p_meshes[0].m_primitives[0];
    u32 vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    newPrim->m_vao = vao;
    newPrim->m_mode = GL_TRIANGLE_STRIP;

    newPrim->m_count = m_indices.size();
    newPrim->m_type = GL_UNSIGNED_INT;
    newPrim->m_offset = 0;

    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 sizeof(u32) * m_indices.size(),
                 m_indices.data(),
                 GL_STATIC_DRAW);
    newPrim->m_ebo = ebo;
    newPrim->m_drawType = 1;

    u32 vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(float) * m_vertices.size() * 3,
                 m_vertices.data(),
                 GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, nullptr);
    glEnableVertexAttribArray(0);
    Primitive::AttribInfo attribInfo;
    attribInfo.vbo = 0;
    attribInfo.type = 3;
    attribInfo.componentType = GL_FLOAT;
    attribInfo.normalized = GL_FALSE;
    attribInfo.byteStride = 3 * sizeof(float);
    attribInfo.byteOffset = 0;
    newPrim->attributes["POSITION"] = attribInfo;
  }

  stbi_image_free(imageData);
}
