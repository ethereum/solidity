interface I {
    function f() external;
}
contract C {
    function f() external {}
    bytes4 constant functionSelector1 = I.f.selector;
    bytes4 constant functionSelector2 = this.f.selector;
}
// ----
