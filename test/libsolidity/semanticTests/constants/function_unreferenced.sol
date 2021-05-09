contract B {
    function g() public {}
}
contract C is B {
    bytes4 constant s2 = B.g.selector;
    function f() external pure returns (bytes4) { return s2; }
}
// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// f() -> 0xe2179b8e00000000000000000000000000000000000000000000000000000000
