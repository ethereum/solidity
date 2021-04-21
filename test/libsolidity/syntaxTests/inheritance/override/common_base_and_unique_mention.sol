interface I {
    function f() external;
    function g() external;
}
abstract contract A is I {
    function f() external override {}
}
abstract contract B is I {
    function g() external override {}
}
contract C is A, B {
}
// ----
