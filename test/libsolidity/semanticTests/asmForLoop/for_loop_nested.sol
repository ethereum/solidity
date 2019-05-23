contract C {
    function f(uint x) public returns (uint i) {
        assembly {
            for {} lt(i, 10) { i := add(i, 1) }
            {
                if eq(x, 0) { i := 2 break }
                for {} lt(x, 3) { i := 17 x := 9 } {
                    if eq(x, 1) { continue }
                    if eq(x, 2) { break }
                }
                if eq(x, 4) { i := 90 }
            }
        }
    }
}
// ====
// compileViaYul: also
// ----
// f(uint256): 0 -> 2
// f(uint256): 1 -> 18
// f(uint256): 2 -> 10
// f(uint256): 4 -> 91
