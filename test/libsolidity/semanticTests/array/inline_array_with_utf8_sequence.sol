pragma abicoder v2;

contract C {
        function f() external pure returns (string[2] memory rdatas) {
                rdatas = [hex'74000001', hex'c4a40001'];
        }
}
// ====
// compileViaYul: also
// ----
// f() -> 0x20, 0x40, 0x80, 4, "t\0\0\x01", 4, "\xc4\xa4\0\x01"
