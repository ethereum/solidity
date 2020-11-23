contract C {
    function div(uint256 a, uint256 b) public returns (uint256) {
        return a / b;
    }

    function mod(uint256 a, uint256 b) public returns (uint256) {
        return a % b;
    }
}
// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// div(uint256,uint256): 7, 2 -> 3
// div(uint256,uint256): 7, 0 -> FAILURE, hex"4e487b71", 0x12 # throws #
// mod(uint256,uint256): 7, 2 -> 1
// mod(uint256,uint256): 7, 0 -> FAILURE, hex"4e487b71", 0x12 # throws #
