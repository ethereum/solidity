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
// ----
// div(uint256,uint256): 7, 2 -> 3
// div(uint256,uint256): 7, 0 -> FAILURE # throws #
// mod(uint256,uint256): 7, 2 -> 1
// mod(uint256,uint256): 7, 0 -> FAILURE # throws #
