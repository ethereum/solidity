type Int is int128;
using {
    bitor as |, bitand as &, bitxor as ^, bitnot as ~,
    add as +, sub as -, unsub as -, mul as *, div as /, mod as %,
    eq as ==, noteq as !=, lt as <, gt as >, leq as <=, geq as >=
} for Int;

function uw(Int x) pure returns (int128) {
    return Int.unwrap(x);
}
function w(int128 x) pure returns (Int) {
    return Int.wrap(x);
}
function bitor(Int, Int) pure returns (Int) {
    return w(10);
}
function bitand(Int, Int) pure returns (Int) {
    return w(11);
}
function bitxor(Int, Int) pure returns (Int) {
    return w(12);
}
function bitnot(Int) pure returns (Int) {
    return w(13);
}
function add(Int x, Int) pure returns (Int) {
    return w(uw(x) + 10);
}
function sub(Int, Int) pure returns (Int) {
    return w(15);
}
function unsub(Int) pure returns (Int) {
    return w(16);
}
function mul(Int, Int) pure returns (Int) {
    return w(17);
}
function div(Int, Int) pure returns (Int) {
    return w(18);
}
function mod(Int, Int) pure returns (Int) {
    return w(19);
}
function eq(Int x, Int) pure returns (bool) {
    return uw(x) == 1;
}
function noteq(Int x, Int) pure returns (bool) {
    return uw(x) == 2;
}
function lt(Int x, Int) pure returns (bool) {
    return uw(x) < 10;
}
function gt(Int x, Int) pure returns (bool) {
    return uw(x) > 10;
}
function leq(Int x, Int) pure returns (bool) {
    return uw(x) <= 10;
}
function geq(Int x, Int) pure returns (bool) {
    return uw(x) >= 10;
}

contract C {
    function test_bitor() public pure returns (Int) { return w(1) | w(2); }
    function test_bitand() public pure returns (Int) { return w(1) | w(2); }
    function test_bitxor() public pure returns (Int) { return w(1) ^ w(2); }
    function test_bitnot() public pure returns (Int) { return ~w(1); }
    function test_add(int128 x) public pure returns (Int) { return w(x) + w(2); }
    function test_sub() public pure returns (Int) { return w(1) - w(2); }
    function test_unsub() public pure returns (Int) { return -w(1); }
    function test_mul() public pure returns (Int) { return w(1) * w(2); }
    function test_div() public pure returns (Int) { return w(1) / w(2); }
    function test_mod() public pure returns (Int) { return w(1) % w(2); }
    function test_eq(int128 x) public pure returns (bool) { return w(x) == w(2); }
    function test_neq(int128 x) public pure returns (bool) { return w(x) != w(2); }
    function test_lt(int128 x) public pure returns (bool) { return w(x) < w(2); }
    function test_gt(int128 x) public pure returns (bool) { return w(x) > w(2); }
    function test_leq(int128 x) public pure returns (bool) { return w(x) <= w(2); }
    function test_geq(int128 x) public pure returns (bool) { return w(x) >= w(2); }
}

// ====
// compileViaYul: also
// ----
// test_bitor() -> 10
// test_bitand() -> 10
// test_bitxor() -> 12
// test_bitnot() -> 13
// test_add(int128): 4 -> 14
// test_add(int128): 104 -> 114
// test_sub() -> 15
// test_unsub() -> 16
// test_mul() -> 17
// test_div() -> 18
// test_mod() -> 19
// test_eq(int128): 1 -> true
// test_eq(int128): 2 -> false
// test_neq(int128): 2 -> true
// test_neq(int128): 1 -> false
// test_lt(int128): 9 -> true
// test_lt(int128): 10 -> false
// test_gt(int128): 11 -> true
// test_gt(int128): 10 -> false
// test_leq(int128): 10 -> true
// test_leq(int128): 11 -> false
// test_geq(int128): 10 -> true
// test_geq(int128): 9 -> false
