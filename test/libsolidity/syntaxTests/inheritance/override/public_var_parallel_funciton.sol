interface A {
    function foo() external returns (uint);
}
contract B {
    uint public foo;
}
contract X is A, B {
}
// ----
// TypeError 6480: (96-118): Derived contract must override function "foo". Two or more base classes define function with same name and parameter types. Since one of the bases defines a public state variable which cannot be overridden, you have to change the inheritance layout or the names of the functions.
