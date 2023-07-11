contract C {
    function f(uint256 a) public returns (uint256 b) {
        assembly {
            function fac(n) -> nf {
                switch n
                    case 0 {
                        nf := 1
                    }
                    case 1 {
                        nf := 1
                    }
                    default {
                        nf := mul(n, fac(sub(n, 1)))
                    }
            }
            b := fac(a)
        }
    }
}
// ----
// f(uint256): 0 -> 1
// f(uint256): 1 -> 1
// f(uint256): 2 -> 2
// f(uint256): 3 -> 6
// f(uint256): 4 -> 24
