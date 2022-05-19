contract C {
    function memorySize() internal pure returns (uint s) {
        assembly { s := mload(0x40) }
    }
    function f() public returns (uint, uint, uint) {
        uint a = memorySize();
        g();
        uint b = memorySize();
        h();
        uint c = memorySize();
        i();
        uint d = memorySize();
        return (b - a, c - b, d - c);
    }
    // In these functions, we do allocate memory in both cases.
    // In `i()`, this could be avoided but we would have to check
    // that all code paths return explicitly and provide a value.
    function g() internal returns (uint[40] memory) {
    }
    function h() internal returns (uint[40] memory t) {
    }
    function i() internal returns (uint[40] memory) {
        uint[40] memory x;
        return x;
    }
}
// ----
// f() -> 0x0500, 0x0500, 0x0a00
