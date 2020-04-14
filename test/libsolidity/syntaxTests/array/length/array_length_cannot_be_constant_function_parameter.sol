contract C {
    function f(uint constant LEN) public {
        uint[LEN] a;
    }
}
// ----
// DeclarationError: (28-45): The "constant" keyword can only be used for state variables.
// TypeError: (69-72): Invalid array length, expected integer literal or constant expression.
// TypeError: (64-75): Data location must be "storage" or "memory" for variable, but none was given.
