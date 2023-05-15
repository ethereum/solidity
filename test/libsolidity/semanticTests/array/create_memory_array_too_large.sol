contract C {
    function f() public returns (uint256) {
        uint256 l = 2**256 / 32;
        // This used to work without causing an error.
        uint256[] memory x = new uint256[](l);
        uint256[] memory y = new uint256[](1);
        x[1] = 42;
        // This used to overwrite the value written above.
        y[0] = 23;
        return x[1];
    }
    function g() public returns (uint256) {
        uint256 l = 2**256 / 2 + 1;
        // This used to work without causing an error.
        uint16[] memory x = new uint16[](l);
        uint16[] memory y = new uint16[](1);
        x[2] = 42;
        // This used to overwrite the value written above.
        y[0] = 23;
        return x[2];
    }}
// ----
// f() -> FAILURE, hex"4e487b71", 0x41
// g() -> FAILURE, hex"4e487b71", 0x41
