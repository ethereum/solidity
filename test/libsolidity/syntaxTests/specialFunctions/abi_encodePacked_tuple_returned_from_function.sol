contract C {
    function g0() internal pure {}
    function g1() internal pure returns (uint) { return (1); }
    function g2() internal pure returns (uint, uint) { return (2, 3); }

    function h() public pure {
        abi.encodePacked(g0());
        abi.encodePacked(g2());
        abi.encodePacked((g1(), g1()));
    }
}
// ----
// TypeError 2056: (240-244): This type cannot be encoded.
// TypeError 2056: (272-276): This type cannot be encoded.
// TypeError 2056: (304-316): This type cannot be encoded.
