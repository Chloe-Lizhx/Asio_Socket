#pragma once

#include <vector>
#include <memory>

namespace com {
using PtrRequest = std::shared_ptr<Request>;
class Request {

public:
  static void wait(std::vector<PtrRequest> &requests);

  virtual ~Request();

  virtual bool test() = 0;

  virtual void wait() = 0;
};
} // namespace com