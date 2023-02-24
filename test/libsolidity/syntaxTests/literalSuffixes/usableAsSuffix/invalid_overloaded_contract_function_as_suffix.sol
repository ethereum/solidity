contract C {
    uint a = 1000 suffix;

    function suffix(uint) public pure returns (uint) {}
    function suffix(address) public pure returns (uint) {}
}
// ----
// TypeError 9322: (26-37): No matching declaration found after argument-dependent lookup.
