function suffix(uint x) pure returns (uint) { return x; }

contract C {
    uint immutable a = 1 suffix;
}
