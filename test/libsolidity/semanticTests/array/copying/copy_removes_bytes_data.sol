
contract c {
    function set() public returns (bool) { data1 = msg.data; return true; }
    function reset() public returns (bool) { data1 = data2; return true; }
    bytes data1;
    bytes data2;
}
// ====
// compileViaYul: also
// ----
// set(): 1, 2, 3, 4, 5 -> true
// gas irOptimized: 177417
// gas legacy: 177656
// gas legacyOptimized: 177492
// storageEmpty -> 0
// reset() -> true
// storageEmpty -> 1
