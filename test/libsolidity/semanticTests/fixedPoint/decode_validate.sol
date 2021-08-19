contract C {
    function f(fixed8x2) public pure returns (bool) { return true; }
    function g(ufixed64x2) public pure returns (bool) { return true; }
    function h(fixed) public pure returns (bool) { return true; }
    function i(ufixed) public pure returns (bool) { return true; }
}
// ====
// compileViaYul: also
// ----
// f(fixed8x2): 1.27 -> true
// f(fixed8x2): 1.28 -> FAILURE
// f(fixed8x2): -1.27 -> true
// f(fixed8x2): -1.28 -> true
// f(fixed8x2): -1.29 -> FAILURE
// g(ufixed64x2): 184467440737095516.15 -> true
// g(ufixed64x2): 184467440737095516.16 -> FAILURE
// h(fixed128x18): 170141183460469231731.687303715884105727 -> true
// h(fixed128x18): 170141183460469231731.687303715884105728 -> FAILURE
// h(fixed128x18): -170141183460469231731.687303715884105728 -> true
// h(fixed128x18): -170141183460469231731.687303715884105729 -> FAILURE
// i(ufixed128x18): 34028236692093846346.3374607431768211455 -> true
// i(ufixed128x18): 34028236692093846346.3374607431768211456 -> FAILURE
