contract C {
    function f(uint x, uint y) public returns (uint) {
        return x**y;
    }
}
// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// f(uint256,uint256): 0, 0 -> 1
// f(uint256,uint256): 0, 1 -> 0x00
// f(uint256,uint256): 0, 2 -> 0x00
// f(uint256,uint256): 1, 0 -> 1
// f(uint256,uint256): 1, 1 -> 1
// f(uint256,uint256): 1, 2 -> 1
// f(uint256,uint256): 2, 0 -> 1
// f(uint256,uint256): 2, 1 -> 2
// f(uint256,uint256): 2, 2 -> 4
// f(uint256,uint256): 7, 63 -> 174251498233690814305510551794710260107945042018748343
// f(uint256,uint256): 128, 2 -> 0x4000
