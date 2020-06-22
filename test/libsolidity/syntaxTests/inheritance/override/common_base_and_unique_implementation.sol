interface I {
    function f() external;
    function g() external;
}
abstract contract A is I {
    function f() external override {}
    function g() external override virtual;
}
abstract contract B is I {
    function g() external override {}
    function f() external override virtual;
}
contract C is A, B {
}
// ----
// TypeError 6480: (292-314): Derived contract must override function "f". Two or more base classes define function with same name and parameter types.
// TypeError 6480: (292-314): Derived contract must override function "g". Two or more base classes define function with same name and parameter types.
