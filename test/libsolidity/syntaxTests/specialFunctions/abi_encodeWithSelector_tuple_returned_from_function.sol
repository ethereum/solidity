contract C {
    function g0() internal pure {}
    function g1() internal pure returns (uint) { return (1); }
    function g2() internal pure returns (uint, uint) { return (2, 3); }

    function f0() public {}
    function f1(uint) public {}
    function f2(uint, uint) public {}

    function h() public pure {
        abi.encodeWithSelector(this.f0.selector, g0());
        abi.encodeWithSelector(this.f0.selector, g2());
        abi.encodeWithSelector(this.f0.selector, (g1(), g1()));
    }
}
// ----
// TypeError 2056: (363-367): This type cannot be encoded.
// TypeError 2056: (419-423): This type cannot be encoded.
// TypeError 2056: (475-487): This type cannot be encoded.
