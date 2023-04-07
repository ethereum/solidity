library L {
    function suffix(uint8) internal pure returns (uint8) {}
    function suffix(uint16) internal pure returns (bytes16) {}
}

contract C {
    using L for uint8;

    function f(uint8 x) public {
        1 x.suffix;
    }
}
// ----
// TypeError 6675: (218-226): Member "suffix" not unique after argument-dependent lookup in uint8.
