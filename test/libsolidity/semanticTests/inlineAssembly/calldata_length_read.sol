contract C {
    function lenBytesRead(bytes calldata x) public returns (uint l) {
        assembly { l := x.length }
    }

    function lenStringRead(string calldata x) public returns (uint l) {
        assembly { l := x.length }
    }
}
// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// lenBytesRead(bytes): 0x20, 4, "abcd" -> 4
// lenBytesRead(bytes): 0x20, 0, "abcd" -> 0x00
// lenBytesRead(bytes): 0x20, 0x21, "abcd", "ef" -> 33
// lenStringRead(string): 0x20, 4, "abcd" -> 4
// lenStringRead(string): 0x20, 0, "abcd" -> 0x00
// lenStringRead(string): 0x20, 0x21, "abcd", "ef" -> 33
