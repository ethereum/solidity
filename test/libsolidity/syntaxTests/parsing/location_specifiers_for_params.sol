contract Foo {
    function f(uint[] storage constant x, uint[] memory y) internal { }
}
// ----
// DeclarationError 1788: (30-55): The "constant" keyword can only be used for state variables or variables at file level.
// TypeError 9259: (30-55): Only constants of value type and byte array type are implemented.
