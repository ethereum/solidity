contract Foo {
    function f(uint[] storage constant x, uint[] memory y) internal { }
}
// ----
// DeclarationError 1788: (30-55): The "constant" keyword can only be used for state variables or variables at file level.
// DeclarationError 9259: (30-55): Constants of non-value type not yet implemented.
