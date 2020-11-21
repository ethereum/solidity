contract C {
    function f(uint256 a) public returns (uint256 b) {
        assembly {
            function fac(n) -> nf {
                nf := 1
                for {
                    let i := n
                } gt(i, 0) {
                    i := sub(i, 1)
                } {
                    nf := mul(nf, i)
                }
            }
            b := fac(a)
        }
    }
}

// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// f(uint256): 0 -> 1
// f(uint256): 1 -> 1
// f(uint256): 2 -> 2
// f(uint256): 3 -> 6
// f(uint256): 4 -> 24
