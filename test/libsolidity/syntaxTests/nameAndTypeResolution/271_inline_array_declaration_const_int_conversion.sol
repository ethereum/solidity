contract C {
    function f() public returns (uint) {
        uint8[4] memory z = [1,2,3,5];
        return (z[0]);
    }
}
// ----
// Warning: (17-121): Function state mutability can be restricted to pure
