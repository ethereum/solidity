contract C {
    modifier repeat(bool twice) {
        if (twice) _;
        _;
    }

    function f(bool twice) public repeat(twice) returns (uint256 r) {
        r += 1;
    }
}
// ====
// compileViaYul: true
// ----
// f(bool): false -> 1
// f(bool): true -> 1
