#include <exception>
#include <iostream>
#include <memory>
#include <stdexcept>

#include "Game/Window.hpp"

int main() {
  if (Window::open()) {
    return Window::start();
  }
  return -1;
}
