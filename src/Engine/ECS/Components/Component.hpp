#ifndef COMPONENT_H_
#define COMPONENT_H_

#include <bitset>
#include <cstddef>



class Component {
public:
  Component() = default;
  virtual ~Component() {}
};

template <typename T> class ComponentHandle {
public:
  ComponentHandle(T *component) : component_(component) {}
  T *operator->() const { return component_; }
  T &operator*() const { return *component_; }
  operator bool() const { return component_ != nullptr; }

private:
  T *component_;
};

#endif // COMPONENT_H_
