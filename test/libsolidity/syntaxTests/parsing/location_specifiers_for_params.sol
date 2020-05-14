contract Foo {
    function f(uint[] storage constant x, uint[] memory y) internal { }
}
// ----
// DeclarationError: (30-55): The "constant" keyword can only be used for state variables.
