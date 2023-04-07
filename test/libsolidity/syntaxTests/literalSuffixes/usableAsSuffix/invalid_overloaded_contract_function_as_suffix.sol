contract C {
    uint a = 1000 suffix;

    function suffix(uint) public pure returns (uint) {}
    function suffix(address) public pure returns (uint) {}
}
// ----
// TypeError 2144: (31-37): No matching declaration found after variable lookup.
