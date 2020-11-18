#pragma once

#include <string>

namespace YAML {

class AbstractEventObserver {
public:
    virtual ~AbstractEventObserver() = default;

    virtual void newMapItem(const std::string& name, int spaces) = 0;
    virtual void newScalar(const std::string& name) = 0;
    virtual void newSequenceItem(const std::string& value, int spaces) = 0;
};

}
