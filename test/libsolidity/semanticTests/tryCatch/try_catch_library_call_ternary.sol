library L {
    function f(uint t, bool b) public pure returns (uint) {
        if (b) {
            return t + 1;
        } else {
            revert("function f reverted");
        }
    }
    function g(uint t, bool b) external pure returns (uint) {
        if (b) {
            return t + 2;
        } else {
            revert("function g reverted");
        }
    }
}
contract C {
    function test(bool b1, bool b2) public returns (uint, string memory) {
        try (b1 ? L.f : L.g)(7, b2) returns (uint x) {
            return (x, "success");
        } catch Error(string memory message) {
            return (0, message);
        }
    }
}
// ====
// EVMVersion: >=byzantium
// ----
// library: L
// test(bool,bool): true, true -> 8, 0x40, 7, "success"
// test(bool,bool): true, false -> 0, 0x40, 0x13, "function f reverted"
// test(bool,bool): false, true -> 9, 0x40, 7, "success"
// test(bool,bool): false, false -> 0, 0x40, 0x13, "function g reverted"
