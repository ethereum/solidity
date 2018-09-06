contract test {
    function f(uint[] memory constant a) public { }
}
// ----
// DeclarationError: (31-55): The "constant" keyword can only be used for state variables.
// TypeError: (31-55): Constants of non-value type not yet implemented.
// TypeError: (31-55): Uninitialized "constant" variable.
