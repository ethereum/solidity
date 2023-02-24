library L {
    function suffix(uint x, uint y) internal pure returns (uint) { return x + y; }
}

contract C {
    using L for uint;

    uint a = 42;
    uint b = 1000 a.suffix;
}
// ----
// TypeError 4438: (164-177): The literal suffix must be either a subdenomination or a file-level suffix function.
