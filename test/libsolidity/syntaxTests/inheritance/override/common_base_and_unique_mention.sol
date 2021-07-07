interface I {
    function f() external;
    function g() external;
}
abstract contract A is I {
    function f() external {}
}
abstract contract B is I {
    function g() external {}
}
contract C is A, B {
}
// ----
