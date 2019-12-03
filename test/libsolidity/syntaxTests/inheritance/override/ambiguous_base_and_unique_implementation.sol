interface I {
    function f() external;
    function g() external;
}
interface J {
	function f() external;
}
abstract contract A is I, J {
    function f() external override (I, J) {}
    function g() external override virtual;
}
abstract contract B is I {
    function f() external override virtual;
    function g() external override {}
}
contract C is A, B {
}
// ----
// TypeError: (342-364): Derived contract must override function "f". Function with the same name and parameter types defined in two or more base classes.
// TypeError: (342-364): Derived contract must override function "g". Function with the same name and parameter types defined in two or more base classes.
