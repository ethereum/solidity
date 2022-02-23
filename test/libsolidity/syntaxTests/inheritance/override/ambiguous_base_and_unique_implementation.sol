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
// TypeError 6480: (342-364): Derived contract must override function "f". Two or more base classes define function with same name and parameter types.
// TypeError 6480: (342-364): Derived contract must override function "g". Two or more base classes define function with same name and parameter types.
