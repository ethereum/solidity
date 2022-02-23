abstract contract A {
    function f() external {}
    function g() external virtual;
}
abstract contract B {
    function g() external {}
    function f() external virtual;
}
contract C is A, B {
}
// ----
// TypeError 6480: (176-198): Derived contract must override function "f". Two or more base classes define function with same name and parameter types.
// TypeError 6480: (176-198): Derived contract must override function "g". Two or more base classes define function with same name and parameter types.
