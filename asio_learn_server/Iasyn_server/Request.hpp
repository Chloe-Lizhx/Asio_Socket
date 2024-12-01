#pragma once

#include <vector>
#include <memory>
#include "SharedPointer.hpp"

namespace com {
class Request {

public:
  static void wait(std::vector<RequestPtr> &requests);

  virtual ~Request();

  virtual bool test() = 0;

  virtual void wait() = 0;
};
} // namespace com