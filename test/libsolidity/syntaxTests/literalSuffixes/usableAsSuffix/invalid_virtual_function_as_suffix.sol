abstract contract B {
    function suffix(uint x) internal pure virtual returns (uint);
}

contract C is B {
    uint a = 1000 suffix;

    function suffix(uint x) internal pure override returns (uint) { return x; }
}
// ----
// TypeError 4438: (122-133): The literal suffix must be either a subdenomination or a file-level suffix function.
