#ifndef COMPONENT_H_
#define COMPONENT_H_

using ComponentType = std::size_t;

// Basic component interface which all other components implement
template <typename T> class Component {
public:
  Component() = default;
  virtual ~Component() = default;

  ComponentType getComponentType() { return m_type; }

protected:
  ComponentType m_type = m_nextComponentType++;
};

static ComponentType m_nextComponentType = 0;

#endif // COMPONENT_H_
