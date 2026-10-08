library L {
    function f() external returns (uint256) { return 1; }
    function g() external pure returns (uint256) { return 2; }
}

contract C {
    function test(bool b) public returns (uint256) {
        return (b ? L.f : L.g)();
    }
}
// ----
// library: L
// test(bool): true -> 1
// test(bool): false -> 2
