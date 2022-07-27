function suffix(uint x, uint y) pure returns (uint) { return x + y; }

contract C {
    using {suffix} for uint;

    uint a = 42;
    uint b = 1000 a.suffix;
}
// ----
// TypeError 4438: (144-157): The literal suffix needs to be a pre-defined suffix or a file-level pure function.
