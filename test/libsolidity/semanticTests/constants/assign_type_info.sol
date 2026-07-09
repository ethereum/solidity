contract A {
}

contract B {
    bytes constant creationCode = type(A).creationCode;
    bytes constant runtimeCode = type(A).runtimeCode;

    function nonEmptyCode() public pure returns (bool) {
        return creationCode.length > 0 && runtimeCode.length > 0;
    }
}
// ====
// targetContract: B
// ----
// nonEmptyCode() -> true
