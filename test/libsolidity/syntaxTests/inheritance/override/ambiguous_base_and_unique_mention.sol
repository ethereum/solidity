interface I {
    function f() external;
    function g() external;
}
interface J {
	function f() external;
}
abstract contract A is I, J {
    function f() external override (I, J) {}
}
abstract contract B is I {
    function g() external override {}
}
contract C is A, B {
}
// ----
// TypeError 6480: (254-276): Derived contract must override function "f". Two or more base classes define function with same name and parameter types.
