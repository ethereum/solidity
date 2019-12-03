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
// TypeError: (176-198): Derived contract must override function "f". Function with the same name and parameter types defined in two or more base classes.
// TypeError: (176-198): Derived contract must override function "g". Function with the same name and parameter types defined in two or more base classes.
