contract test {
    function f(uint[] memory constant a) public { }
}
// ----
// DeclarationError 1788: (31-55): The "constant" keyword can only be used for state variables or variables at file level.
// TypeError 9259: (31-55): Only constants of value type and byte array type are implemented.
