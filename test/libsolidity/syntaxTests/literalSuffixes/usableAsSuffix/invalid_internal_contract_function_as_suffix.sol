contract B {
    function inheritedSuffix(uint x) internal pure returns (uint) { return x; }
}

contract C is B {
    uint a = 1000 suffix;
    uint b = 1000 B.inheritedSuffix;
    uint c = 1000 C.suffix;

    function suffix(uint x) internal pure returns (uint) { return x; }
}
// ----
// TypeError 4438: (132-138): The literal suffix must be either a subdenomination or a file-level suffix function.
// TypeError 4438: (158-175): The literal suffix must be either a subdenomination or a file-level suffix function.
// TypeError 4438: (195-203): The literal suffix must be either a subdenomination or a file-level suffix function.
