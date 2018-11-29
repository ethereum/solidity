contract C {
    function f() public returns (string memory) {
        return (["foo", "man", "choo"][1]);
    }
}
// ----
// Warning: (17-112): Function state mutability can be restricted to pure
