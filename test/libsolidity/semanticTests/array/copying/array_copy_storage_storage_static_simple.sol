contract C {
    bytes1[2] data1;
    bytes2[2] data2;
    function test() public returns (bytes2, bytes2) {
        uint i;
        for (i = 0; i < data1.length; ++i)
            data1[i] = bytes1(uint8(1 + i));
        data2 = data1;
        return (data2[0], data2[1]);
    }
}
// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// test() -> left(0x01), left(0x02)
// gas legacy: 90001
// gas legacyOptimized: 89055
