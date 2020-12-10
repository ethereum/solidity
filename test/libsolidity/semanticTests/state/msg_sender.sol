contract C {
    function f() public returns (address payable) {
        return msg.sender;
    }
}
// ====
// compileViaYul: also
// ----
// f() -> 0x1212121212121212121212121212120000000012
