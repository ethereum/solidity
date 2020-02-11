contract test {
    function f(uint a) public returns(uint d) {
        return 2 ** a;
    }
}

// ----
// f(uint256): 0 -> 1
// f(uint256): 1 -> 2
// f(uint256): 2 -> 4
// f(uint256): 3 -> 8
// f(uint256): 4 -> 16
// f(uint256): 5 -> 32
// f(uint256): 6 -> 64
// f(uint256): 7 -> 128
// f(uint256): 8 -> 256
// f(uint256): 9 -> 512
// f(uint256): 10 -> 1024
// f(uint256): 11 -> 2048
// f(uint256): 12 -> 4096
// f(uint256): 13 -> 8192
// f(uint256): 14 -> 16384
// f(uint256): 15 -> 32768
