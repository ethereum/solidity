contract A {
}

contract C {
    function f() public pure returns (bytes memory) {
        return type(A).runtimeCode;
    }
}
// ----
