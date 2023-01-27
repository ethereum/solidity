contract C {
    function f() public returns (string memory) {
        try this.f() returns (string storage a) {} catch { }
    }
}
// ----
// TypeError 6651: (93-109): Data location must be "memory" for parameter in function, but "storage" was given.
