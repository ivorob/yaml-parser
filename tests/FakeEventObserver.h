#pragma once

#include <map>
#include <vector>

#include "AbstractEventObserver.h"

namespace Fake {

class EventObserver : public YAML::AbstractEventObserver {
public:
    class Value {
    public:
        Value();
        Value(const std::string& value, int spaces);

        void setValue(const std::string& value);
        const std::string& getValue() const;

        int getSpaces() const;
    private:
        std::string value;
        int spaces;
    };
public:
    void newMapItem(const std::string& name, int spaces) override;
    void newScalar(const std::string& scalar) override;
    void newSequenceItem(int spaces) override;
public:
    std::map<std::string, Value> events;
    std::vector<Value> sequences;
private:
    std::string lastAddedMapItem;
    bool foundSequence = false;
};

}
