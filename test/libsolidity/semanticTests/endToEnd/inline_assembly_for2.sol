contract C {
    uint st;

    function f(uint a) public returns(uint b, uint c, uint d) {
        st = 0;
        assembly {
            function sideeffect(r) - > x {
                sstore(0, add(sload(0), r)) x := 1
            }
            for {
                let i := a
            }
            eq(i, sideeffect(2)) {
                d := add(d, 3)
            } {
                b := i
                i := 0
            }
        }
        c = st;
    }
}

// ====
// compileViaYul: also
// ----
// f(uint256): 0 -> 0, 2, 0
// f(uint256): 1 -> 1, 4, 3
// f(uint256): 2 -> 0, 2, 0
