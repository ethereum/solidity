function suffix(uint x) pure suffix returns (uint) { return x; }

contract C {
    uint immutable a = 1 suffix;
}
