contract C {
    function g0() internal pure {}
    function g1() internal pure returns (uint) { return (1); }
    function g2() internal pure returns (uint, uint) { return (2, 3); }

    function h() public pure {
        abi.encodeWithSignature("f0()", g0());
        abi.encodeWithSignature("f()", g2());
        abi.encodeWithSignature("f()", (g1(), g1()));
    }
}
// ----
// TypeError 2056: (255-259): This type cannot be encoded.
// TypeError 2056: (301-305): This type cannot be encoded.
// TypeError 2056: (347-359): This type cannot be encoded.
