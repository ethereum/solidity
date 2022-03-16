contract c {
    function set(uint key) public returns (bool) { data[key] = msg.data; return true; }
    function copy(uint from, uint to) public returns (bool) { data[to] = data[from]; return true; }
    mapping(uint => bytes) data;
}
// ====
// compileViaYul: also
// ----
// set(uint256): 1, 2 -> true
// gas irOptimized: 110604
// gas legacy: 111088
// gas legacyOptimized: 110733
// set(uint256): 2, 2, 3, 4, 5 -> true
// gas irOptimized: 177564
// gas legacy: 178018
// gas legacyOptimized: 177663
// storageEmpty -> 0
// copy(uint256,uint256): 1, 2 -> true
// storageEmpty -> 0
// copy(uint256,uint256): 99, 1 -> true
// storageEmpty -> 0
// copy(uint256,uint256): 99, 2 -> true
// storageEmpty -> 1
