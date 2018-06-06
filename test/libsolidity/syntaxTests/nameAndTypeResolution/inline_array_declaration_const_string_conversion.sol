contract C {
    function f() public returns (string) {
        string[2] memory z = ["Hello", "World"];
        return (z[0]);
    }
}
// ----
// Warning: (17-133): Function state mutability can be restricted to pure
