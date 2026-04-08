#pragma once

#include "Preprocessing/MriPreprocessingPipeline.h"

class DWISkullExtractionStage final : public IMriPreprocessingStage
{
public:
  const char *Name() const override;
  void Execute(MriPreprocessingContext &context) const override;
};

std::unique_ptr<IMriPreprocessingStage> CreateDwiSkullExtractionStage();
