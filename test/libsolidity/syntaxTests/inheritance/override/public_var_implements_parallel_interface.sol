interface A {
    function foo() external returns (uint);
    function goo() external returns (uint);
}
interface B {
    function foo() external returns (uint);
    function goo() external returns (uint);
}
contract X is A, B {
	uint public override(A, B) foo;
    function goo() external virtual override(A, B) returns (uint) {}
}
abstract contract T is A {
    function foo() external virtual override returns (uint);
    function goo() external virtual override returns (uint);
}
contract Y is X, T {
}
// ----
// TypeError 6480: (484-506): Derived contract must override function "foo". Two or more base classes define function with same name and parameter types. Since one of the bases defines a public state variable which cannot be overridden, you have to change the inheritance layout or the names of the functions.
// TypeError 6480: (484-506): Derived contract must override function "goo". Two or more base classes define function with same name and parameter types.
