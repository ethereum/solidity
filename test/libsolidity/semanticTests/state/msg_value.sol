contract C {
    function f() public payable returns (uint) {
        return msg.value;
    }
}
// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// f() -> 0
// f(), 12 ether -> 12000000000000000000
