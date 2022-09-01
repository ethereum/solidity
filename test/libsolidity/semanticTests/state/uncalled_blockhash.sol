contract C {
    function f() public returns (bytes32) {
        return (blockhash)(block.number - 1);
    }
}
// ====
// compileToEwasm: also
// ----
// f() -> 0x3737373737373737373737373737373737373737373737373737373737373738
