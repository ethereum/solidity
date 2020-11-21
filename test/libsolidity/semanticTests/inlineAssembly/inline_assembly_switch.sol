contract C {
    function f(uint256 a) public returns (uint256 b) {
        assembly {
            switch a
                case 1 {
                    b := 8
                }
                case 2 {
                    b := 9
                }
                default {
                    b := 2
                }
        }
    }
}

// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// f(uint256): 0 -> 2
// f(uint256): 1 -> 8
// f(uint256): 2 -> 9
// f(uint256): 3 -> 2
