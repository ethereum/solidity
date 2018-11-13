contract C {
    function f() public returns (string memory) {
        string memory x = "Hello";
        string memory y = "World";
        string[2] memory z = [x, y];
        return (z[0]);
    }
}
// ----
// Warning: (17-198): Function state mutability can be restricted to pure
