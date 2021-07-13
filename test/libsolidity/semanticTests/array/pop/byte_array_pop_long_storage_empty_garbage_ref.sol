contract c {
    uint256 a;
    uint256 b;
    bytes data;
    function test() public {
        for (uint8 i = 0; i <= 40; i++)
            data.push(0x03);
        for (uint8 j = 0; j <= 40; j++) {
            assembly {
                mstore(0, "garbage")
            }
            data.pop();
        }
    }
}
// ====
// compileViaYul: also
// ----
// test() ->
// gas irOptimized: 142637
// gas legacy: 165363
// gas legacyOptimized: 159446
// storageEmpty -> 1
