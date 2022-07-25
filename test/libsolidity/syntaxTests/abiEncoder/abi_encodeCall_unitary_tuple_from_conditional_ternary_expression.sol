contract C {
    function g1() internal pure returns (uint) { return (1); }

    function f1(uint) public {}

    function h() public view {
        abi.encodeCall(this.f1, true ? (1) : (2));
        abi.encodeCall(this.f1, true ? g1() : g1());
    }
}
// ----
