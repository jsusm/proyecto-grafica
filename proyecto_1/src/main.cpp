#include <iostream>

class Figure {
public:
  virtual void sayHello() {};
};

class Rect: public Figure {
public:
  void sayHello() {
    std::cout << "hello world\n";
  }
};


int main(){
  Figure* rect = new Rect();
  rect->sayHello();
  return 0;
}
