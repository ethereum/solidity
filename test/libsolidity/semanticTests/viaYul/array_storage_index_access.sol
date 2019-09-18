contract C {
    uint[] storageArray;
    function test_indices(uint256 len) public
    {
        storageArray = new uint[](len);
        for (uint i = 0; i < len; i++)
            storageArray[i] = i + 1;

        for (uint i = 0; i < len; i++)
            require(storageArray[i] == i + 1);
    }
}
// ----
// test_indices(uint256): 1 ->
// test_indices(uint256): 129 ->
// test_indices(uint256): 5 ->
// test_indices(uint256): 10 ->
// test_indices(uint256): 15 ->
// test_indices(uint256): 0xFF ->
// test_indices(uint256): 1000 ->
// test_indices(uint256): 129 ->
// test_indices(uint256): 128 ->
// test_indices(uint256): 1 ->
