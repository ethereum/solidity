
contract c {
    uint[] data1;
    uint[] data2;
    function setData1(uint length, uint index, uint value) public {
        data1 = new uint[](length);
        if (index < length)
            data1[index] = value;
    }
    function copyStorageStorage() public { data2 = data1; }
    function getData2(uint index) public returns (uint len, uint val) {
        len = data2.length; if (index < len) val = data2[index];
    }
}
// ====
// compileViaYul: also
// ----
// setData1(uint256,uint256,uint256): 10, 5, 4 ->
// copyStorageStorage() ->
// gas irOptimized: 111426
// gas legacy: 109278
// gas legacyOptimized: 109268
// getData2(uint256): 5 -> 10, 4
// setData1(uint256,uint256,uint256): 0, 0, 0 ->
// copyStorageStorage() ->
// getData2(uint256): 0 -> 0, 0
// storageEmpty -> 1
