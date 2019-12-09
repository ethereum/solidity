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
// TypeError: (292-314): Derived contract must override function "f". Function with the same name and parameter types defined in two or more base classes.
// TypeError: (292-314): Derived contract must override function "g". Function with the same name and parameter types defined in two or more base classes.
