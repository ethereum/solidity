contract C {
    function f() public returns (uint i) {
        assembly {
            for {} lt(i, 10) { i := add(i, 1) }
            {
                if eq(i, 6) { break }
                i := add(i, 1)
            }
        }
    }
}
// ====
// compileViaYul: also
// ----
// f() -> 6
