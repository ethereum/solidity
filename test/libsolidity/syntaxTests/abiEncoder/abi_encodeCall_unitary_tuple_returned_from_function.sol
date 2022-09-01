contract C {
    function g1() internal pure returns (uint) { return (1); }
    function g2() internal pure returns (uint, uint) { return (2, 3); }

    function f1(uint) public {}
    function f2(uint, uint) public {}

    function h() public view {
        abi.encodeCall(this.f1, g1());
        abi.encodeCall(this.f1, (g1()));
        abi.encodeCall(this.f2, (g1(), g1()));
    }
}
// ----
