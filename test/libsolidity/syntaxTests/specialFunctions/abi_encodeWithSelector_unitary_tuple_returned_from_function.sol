contract C {
    function g1() internal pure returns (uint) { return (1); }

    function f0() public {}

    function h() public pure {
        abi.encodeWithSelector(this.f0.selector, g1());
    }
}
// ----
