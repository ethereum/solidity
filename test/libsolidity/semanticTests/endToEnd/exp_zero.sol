contract test {
    function f(uint a) public returns(uint d) {
        return a ** 0;
    }
}

// ----
// f(uint256): 0 -> 1
// f(uint256): 1 -> 1
// f(uint256): 2 -> 1
// f(uint256): 3 -> 1
// f(uint256): 4 -> 1
// f(uint256): 5 -> 1
// f(uint256): 6 -> 1
// f(uint256): 7 -> 1
// f(uint256): 8 -> 1
// f(uint256): 9 -> 1
// f(uint256): 10 -> 1
// f(uint256): 11 -> 1
// f(uint256): 12 -> 1
// f(uint256): 13 -> 1
// f(uint256): 14 -> 1
// f(uint256): 15 -> 1
