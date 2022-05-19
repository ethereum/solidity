pragma abicoder v2;
contract C {
    event E(uint[]);
    function createEvent(uint x) public {
        uint[] memory arr = new uint[](3);
        arr[0] = x;
        arr[1] = x + 1;
        arr[2] = x + 2;
        emit E(arr);
    }
}
// ----
// createEvent(uint256): 42 ->
// ~ emit E(uint256[]): 0x20, 0x03, 0x2a, 0x2b, 0x2c
