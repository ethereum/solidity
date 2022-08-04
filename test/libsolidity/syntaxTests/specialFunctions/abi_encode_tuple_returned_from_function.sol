contract C {
    function g0() internal pure {}
    function g1() internal pure returns (uint) { return (1); }
    function g2() internal pure returns (uint, uint) { return (2, 3); }

    function h() public pure {
        abi.encode(g0());
        abi.encode(g2());
        abi.encode((g1(), g1()));
    }
}
// ----
// TypeError 2056: (234-238): This type cannot be encoded.
// TypeError 2056: (260-264): This type cannot be encoded.
// TypeError 2056: (286-298): This type cannot be encoded.
