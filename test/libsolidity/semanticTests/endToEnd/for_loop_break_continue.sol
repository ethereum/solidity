contract test {
    function f(uint n) public returns(uint r) {
        uint i = 1;
        uint k = 0;
        for (i *= 5; k < n; i *= 7) {
            k++;
            i += 4;
            if (n % 3 == 0)
                break;
            i += 9;
            if (n % 2 == 0)
                continue;
            i += 19;
        }
        return i;
    }
}

// ----
// f(uint256): 0 -> 5
// f(uint256): 1 -> 259
// f(uint256): 2 -> 973
// f(uint256): 3 -> 9
// f(uint256): 4 -> 48405
// f(uint256): 5 -> 711459
// f(uint256): 6 -> 9
// f(uint256): 7 -> 34863283
// f(uint256): 8 -> 116256805
// f(uint256): 9 -> 9
