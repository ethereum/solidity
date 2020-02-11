contract test {
    function f(uint x) public returns(uint y) {
        while (x > 1) {
            if (x == 10) break;
            while (x > 5) {
                if (x == 8) break;
                x--;
                if (x == 6) continue;
                return x;
            }
            x--;
            if (x == 3) continue;
            break;
        }
        return x;
    }
}

// ====
// compileViaYul: also
// ----
// f(uint256): 0 -> 0
// f(uint256): 1 -> 1
// f(uint256): 2 -> 1
// f(uint256): 3 -> 2
// f(uint256): 4 -> 2
// f(uint256): 5 -> 4
// f(uint256): 6 -> 5
// f(uint256): 7 -> 5
// f(uint256): 8 -> 7
// f(uint256): 9 -> 8
// f(uint256): 10 -> 10
// f(uint256): 11 -> 10
