pragma abicoder v2;
contract C {
    event E(uint[][]);
    uint[][] arr;
    function createEvent(uint x) public {
        arr.push(new uint[](2));
        arr.push(new uint[](2));
        arr[0][0] = x;
        arr[0][1] = x + 1;
        arr[1][0] = x + 2;
        arr[1][1] = x + 3;
        emit E(arr);
    }
}
// ====
// compileViaYul: also
// ----
// createEvent(uint256): 42 ->
// ~ emit E(uint256[][]): 0x20, 0x02, 0x40, 0xa0, 0x02, 0x2a, 0x2b, 0x02, 0x2c, 0x2d
// gas irOptimized: 185434
// gas legacy: 187621
// gas legacyOptimized: 184551
