interface I {
    event A();
    function f() external;
    function g() external;
    function() external;
}
abstract contract C is I {
    function f() public override {
    }
}
// ----
