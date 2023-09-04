
contract c {
    function set() public returns (bool) { data1 = msg.data; return true; }
    function reset() public returns (bool) { data1 = data2; return true; }
    bytes data1;
    bytes data2;
}
// ----
// set(): 1, 2, 3, 4, 5 -> true
// gas irOptimized: 177376
// gas legacy: 177954
// gas legacyOptimized: 177550
// storageEmpty -> 0
// reset() -> true
// storageEmpty -> 1
