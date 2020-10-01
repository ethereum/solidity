contract c {
    bytes data;

    function foo() public returns (bool) {
        data.push("f");
        data.push("o");
        data.push("o");
        return keccak256(data) == keccak256("foo");
    }
}

// ====
// compileViaYul: also
// ----
// foo() -> true
