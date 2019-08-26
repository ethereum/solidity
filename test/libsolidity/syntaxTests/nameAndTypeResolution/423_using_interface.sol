interface I {
    function f() external;
}
contract C is I {
    function f() public override {
    }
}
// ----
