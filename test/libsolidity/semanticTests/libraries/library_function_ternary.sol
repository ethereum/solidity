library L {
    struct S { uint x; }
    function inc(S storage s, uint v) public returns (uint) { s.x += v; return s.x; }
    function dec(S storage s, uint v) external returns (uint) { s.x -= v; return s.x; }
    function double(uint v) public pure returns (uint) { return 2 * v; }
    function square(uint v) external pure returns (uint) { return v * v; }
}
contract C {
    L.S s;
    function pureSelect(bool b, uint v) public pure returns (uint) {
        return (b ? L.double : L.square)(v);
    }
    function storageSelect(bool b, uint v) public returns (uint, uint) {
        uint r = (b ? L.inc : L.dec)(s, v);
        return (r, s.x);
    }
}
// ====
// EVMVersion: >=byzantium
// ----
// library: L
// pureSelect(bool,uint256): true, 7 -> 14
// pureSelect(bool,uint256): false, 7 -> 49
// storageSelect(bool,uint256): true, 5 -> 5, 5
// storageSelect(bool,uint256): false, 2 -> 3, 3
// storageSelect(bool,uint256): false, 4 -> FAILURE, hex"4e487b71", 0x11
// storageSelect(bool,uint256): true, 1 -> 4, 4
