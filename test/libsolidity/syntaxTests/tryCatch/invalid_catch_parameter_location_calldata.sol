contract C {
    function f() public {
        try this.f() {} catch (string calldata a) { }
    }
}
// ----
// TypeError 6651: (70-87): Data location must be "memory" for parameter in function, but "calldata" was given.
