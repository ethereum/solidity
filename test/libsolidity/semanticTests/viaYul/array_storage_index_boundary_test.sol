contract C {
    uint[] storageArray;
    function test_boundary_check(uint256 len, uint256 access) public returns (uint256)
    {
        // storageArray = new uint[](len);
        while(storageArray.length < len) storageArray.push();
        while(storageArray.length > len) storageArray.pop();
        return storageArray[access];
    }
}
// ====
// compileViaYul: true
// ----
// test_boundary_check(uint256, uint256): 10, 11 -> FAILURE
// test_boundary_check(uint256, uint256): 10, 9 -> 0
// test_boundary_check(uint256, uint256): 1, 9 -> FAILURE
// test_boundary_check(uint256, uint256): 1, 1 -> FAILURE
// test_boundary_check(uint256, uint256): 10, 10 -> FAILURE
// test_boundary_check(uint256, uint256): 256, 256 -> FAILURE
// test_boundary_check(uint256, uint256): 256, 255 -> 0
// test_boundary_check(uint256, uint256): 256, 0xFFFF -> FAILURE
// test_boundary_check(uint256, uint256): 256, 2 -> 0
