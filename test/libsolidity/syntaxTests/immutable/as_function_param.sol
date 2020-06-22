contract C {
    function f(uint immutable) public pure {}
}
// ----
// DeclarationError 8297: (28-42): The "immutable" keyword can only be used for state variables.
