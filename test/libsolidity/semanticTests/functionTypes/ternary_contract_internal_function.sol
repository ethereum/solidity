contract C {
    function f() internal pure returns(uint256) { return 1;}
    function g() internal pure returns(uint256) { return 2; }
    function test(bool b) public returns(uint256) {
        return (b ? C.f : C.g)();
    }
}
// ----
// test(bool): true -> 1
// test(bool): false -> 2
