contract CalldataTest {
    function test(bytes calldata x) public returns (bytes calldata) {
        return x;
    }
    function tester(bytes calldata x) public returns (byte) {
        return this.test(x)[2];
    }
}
// ====
// compileViaYul: also
// EVMVersion: >=byzantium
// ----
// tester(bytes): 0x20, 0x08, "abcdefgh" -> "c"
