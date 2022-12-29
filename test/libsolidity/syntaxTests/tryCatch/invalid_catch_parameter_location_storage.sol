contract C {
    function f() public {
        try this.f() {} catch (string storage a) { }
    }
}
// ----
// TypeError 6651: (70-86): Data location must be "memory" for parameter in function, but "storage" was given.
