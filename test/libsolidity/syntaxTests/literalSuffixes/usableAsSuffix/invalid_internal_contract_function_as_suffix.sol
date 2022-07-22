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
// TypeError 4438: (127-138): The literal suffix needs to be a pre-defined suffix or a file-level pure function.
// TypeError 4438: (153-175): The literal suffix needs to be a pre-defined suffix or a file-level pure function.
// TypeError 4438: (190-203): The literal suffix needs to be a pre-defined suffix or a file-level pure function.
