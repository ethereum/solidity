#pragma once


namespace dev {
namespace solidity {

/// Representation of an interval of source positions.
/// The interval includes start and excludes end.
struct Location {
    Location(int _start, int _end) : start(_start), end(_end) { }
    Location() : start(-1), end(-1) { }

    bool IsValid() const { return start >= 0 && end >= start; }

    int start;
    int end;
};

} }
