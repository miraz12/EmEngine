#include <exception>
#include <iostream>
#include <memory>
#include <stdexcept>

#include "Game/Window.hpp"

void handle_eptr(std::exception_ptr eptr) {
  try {
    if (eptr) {
      std::rethrow_exception(eptr);
    }
  } catch (const std::exception &e) {
    std::cout << "Caught exception \"" << e.what() << "\"\n";
  }
}

int main() {
  std::unique_ptr<int32_t[]> arr(new int32_t[11]);
  arr[10] = 1;
  std::exception_ptr eptr;
  // try {
  if (Window::open()) {
    return Window::start();
  }
  return -1;
  // } catch (...) {
  // eptr = std::current_exception();
  // }
  // handle_eptr(eptr);
}
