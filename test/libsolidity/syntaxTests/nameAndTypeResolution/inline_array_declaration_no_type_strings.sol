contract C {
    function f() public returns (string) {
        return (["foo", "man", "choo"][1]);
    }
}
// ----
// Warning: (17-105): Function state mutability can be restricted to pure
