abstract contract C {
    function suffix(uint8) internal pure virtual returns (uint8);
    function suffix(uint16) internal pure virtual returns (bytes16);

    function f() public {
        1 suffix;
    }
}
// ----
// TypeError 9322: (194-200): No matching declaration found after argument-dependent lookup.
