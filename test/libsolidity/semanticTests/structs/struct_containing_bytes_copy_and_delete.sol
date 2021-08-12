contract c {
    struct Struct { uint a; bytes data; uint b; }
    Struct data1;
    Struct data2;
    function set(uint _a, bytes calldata _data, uint _b) external returns (bool) {
        data1.a = _a;
        data1.b = _b;
        data1.data = _data;
        return true;
    }
    function copy() public returns (bool) {
        data1 = data2;
        return true;
    }
    function del() public returns (bool) {
        delete data1;
        return true;
    }
    function test(uint256 i) public returns (bytes1) {
        return data1.data[i];
    }
}
// ====
// compileViaYul: also
// ----
// storageEmpty -> 1
// set(uint256,bytes,uint256): 12, 0x60, 13, 33, "12345678901234567890123456789012", "3" -> true
// gas irOptimized: 133752
// gas legacy: 134436
// gas legacyOptimized: 133811
// test(uint256): 32 -> "3"
// storageEmpty -> 0
// copy() -> true
// storageEmpty -> 1
// set(uint256,bytes,uint256): 12, 0x60, 13, 33, "12345678901234567890123456789012", "3" -> true
// storageEmpty -> 0
// del() -> true
// storageEmpty -> 1
