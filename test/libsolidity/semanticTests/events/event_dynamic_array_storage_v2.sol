pragma abicoder v2;
contract C {
    event E(uint[]);
    uint[] arr;
    function createEvent(uint x) public {
        while (arr.length < 3)
            arr.push();
        arr[0] = x;
        arr[1] = x + 1;
        arr[2] = x + 2;
        emit E(arr);
    }
}
// ====
// compileViaYul: also
// ----
// createEvent(uint256): 42 ->
// ~ emit E(uint256[]): 0x20, 0x03, 0x2a, 0x2b, 0x2c
// gas irOptimized: 114746
// gas legacy: 116393
// gas legacyOptimized: 114385
