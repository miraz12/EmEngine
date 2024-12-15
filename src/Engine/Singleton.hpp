#ifndef SINGLETON_H_
#define SINGLETON_H_

template<typename T>
class Singleton
{
public:
  static T& getInstance()
  {
    static T instance;
    return instance;
  }

  Singleton(const Singleton&) = delete;
  Singleton& operator=(const Singleton&) = delete;

protected:
  Singleton() = default;
  ~Singleton() = default;
};

#endif // SINGLETON_H_
