contract C {
    function g1() internal pure returns (uint) { return (1); }

    function f1(uint) public {}

    function h() public view {
        uint a;

        abi.encodeCall(this.f1, (a) = g1());
        abi.encodeCall(this.f1, (a) = (1));
    }
}
// ----
