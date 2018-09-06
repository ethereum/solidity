interface I {
    event A();
    function f() external;
    function g() external;
    function() external;
}
contract C is I {
    function f() public {
    }
}
// ----
