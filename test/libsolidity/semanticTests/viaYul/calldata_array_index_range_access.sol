pragma abicoder               v2;
contract C {
    function f(uint256[] calldata x, uint256 s, uint256 e) external returns (uint256) {
        return uint256[](x[s:e]).length;
    }
    function f(uint256[] calldata x, uint256 s, uint256 e, uint256 ss, uint256 ee) external returns (uint256) {
        return uint256[](x[s:e][ss:ee]).length;
    }
    function f_s_only(uint256[] calldata x, uint256 s) external returns (uint256) {
        return uint256[](x[s:]).length;
    }
    function f_e_only(uint256[] calldata x, uint256 e) external returns (uint256) {
        return uint256[](x[:e]).length;
    }
    function g(uint256[] calldata x, uint256 s, uint256 e, uint256 idx) external returns (uint256) {
        return uint256[](x[s:e])[idx];
    }
    function gg(uint256[] calldata x, uint256 s, uint256 e, uint256 idx) external returns (uint256) {
        return x[s:e][idx];
    }
    function gg_s_only(uint256[] calldata x, uint256 s, uint256 idx) external returns (uint256) {
        return x[s:][idx];
    }
    function gg_e_only(uint256[] calldata x, uint256 e, uint256 idx) external returns (uint256) {
        return x[:e][idx];
    }
}
// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// f(uint256[],uint256,uint256): 0x60, 2, 4, 5, 1, 2, 3, 4, 5 -> 2
// f(uint256[],uint256,uint256): 0x60, 2, 6, 5, 1, 2, 3, 4, 5 -> FAILURE
// f(uint256[],uint256,uint256): 0x60, 3, 3, 5, 1, 2, 3, 4, 5 -> 0
// f(uint256[],uint256,uint256): 0x60, 4, 3, 5, 1, 2, 3, 4, 5 -> FAILURE
// f(uint256[],uint256,uint256): 0x60, 0, 3, 5, 1, 2, 3, 4, 5 -> 3
// f(uint256[],uint256,uint256,uint256,uint256): 0xA0, 1, 3, 1, 2, 5, 1, 2, 3, 4, 5 -> 1
// f(uint256[],uint256,uint256,uint256,uint256): 0xA0, 1, 3, 1, 4, 5, 1, 2, 3, 4, 5 -> FAILURE
// f_s_only(uint256[],uint256): 0x40, 2, 5, 1, 2, 3, 4, 5 -> 3
// f_s_only(uint256[],uint256): 0x40, 6, 5, 1, 2, 3, 4, 5 -> FAILURE
// f_e_only(uint256[],uint256): 0x40, 3, 5, 1, 2, 3, 4, 5 -> 3
// f_e_only(uint256[],uint256): 0x40, 6, 5, 1, 2, 3, 4, 5 -> FAILURE
// g(uint256[],uint256,uint256,uint256): 0x80, 2, 4, 1, 5, 1, 2, 3, 4, 5 -> 4
// g(uint256[],uint256,uint256,uint256): 0x80, 2, 4, 3, 5, 1, 2, 3, 4, 5 -> FAILURE, hex"4e487b71", 0x32
// gg(uint256[],uint256,uint256,uint256): 0x80, 2, 4, 1, 5, 1, 2, 3, 4, 5 -> 4
// gg(uint256[],uint256,uint256,uint256): 0x80, 2, 4, 3, 5, 1, 2, 3, 4, 5 -> FAILURE, hex"4e487b71", 0x32
