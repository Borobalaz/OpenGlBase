#pragma once

#include <memory>
#include <string>
#include <vector>

#include "ui/mediator/InspectField.h"

class InspectProvider
{
public:
  virtual ~InspectProvider() = default;

  virtual std::string GetInspectDisplayName() const = 0;
  virtual std::vector<std::shared_ptr<InspectField>> GetInspectFields() = 0;
};
