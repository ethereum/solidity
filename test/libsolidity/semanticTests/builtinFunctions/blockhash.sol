contract C {
    function f() public returns(bytes32) {
        return blockhash(1);
    }
    function g() public returns(bytes32) {
        return blockhash(2);
    }
    function h() public returns(bytes32) {
        return blockhash(3);
    }
}
// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// f() -> 0x3737373737373737373737373737373737373737373737373737373737373738
// g() -> 0x3737373737373737373737373737373737373737373737373737373737373739
// h() -> 0x373737373737373737373737373737373737373737373737373737373737373a
