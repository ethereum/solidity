contract C {
    bytes32 constant x = keccak256("abc");

    function f() public returns (bytes32) {
        return x;
    }
}
// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// f() -> 0x4e03657aea45a94fc7d47ba826c8d667c0d1e6e33a64a036ec44f58fa12d6c45
