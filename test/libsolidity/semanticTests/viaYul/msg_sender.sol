contract C {
    function test() public view returns (bool) {
        address x;
        assembly { x := caller() }
        return x == msg.sender;
    }
}
// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// test() -> true
