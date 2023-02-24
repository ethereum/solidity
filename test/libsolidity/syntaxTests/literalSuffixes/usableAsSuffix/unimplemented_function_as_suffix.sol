abstract contract B {
    uint a = 1000 suffix;

    function suffix(uint x) internal pure virtual returns (uint);
}
// ----
// TypeError 4438: (40-46): The literal suffix must be either a subdenomination or a file-level suffix function.
