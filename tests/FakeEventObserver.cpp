#include "FakeEventObserver.h"

Fake::EventObserver::Value::Value()
    : spaces()
{
}

Fake::EventObserver::Value::Value(const std::string& value, int spaces)
    : value(value),
      spaces(spaces)
{
}

void
Fake::EventObserver::Value::setValue(const std::string& value)
{
    this->value = value;
}

const std::string&
Fake::EventObserver::Value::getValue() const
{
    return this->value;
}

int
Fake::EventObserver::Value::getSpaces() const
{
    return this->spaces;
}

void
Fake::EventObserver::newMapItem(const std::string& name, int spaces)
{
    this->events[name] = Value("", spaces);
    this->lastAddedMapItem = name;
    this->foundSequence = false;
}

void
Fake::EventObserver::newScalar(const std::string& scalar)
{
    if (this->foundSequence && !this->sequences.empty()) {
        auto it = std::rbegin(this->sequences);
        it->setValue(scalar);
    } else if (!this->lastAddedMapItem.empty()) {
        this->events[this->lastAddedMapItem].setValue(scalar);
    }
}

void
Fake::EventObserver::newSequenceItem(int spaces)
{
    this->sequences.emplace_back("", spaces);
    this->foundSequence = true;
}
