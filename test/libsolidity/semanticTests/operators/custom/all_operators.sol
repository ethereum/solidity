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
    return w(1);
}
function bitand(Int, Int) pure returns (Int) {
    return w(2);
}
function bitxor(Int, Int) pure returns (Int) {
    return w(3);
}
function bitnot(Int) pure returns (Int) {
    return w(4);
}
function add(Int, Int) pure returns (Int) {
    return w(5);
}
function sub(Int, Int) pure returns (Int) {
    return w(6);
}
function unsub(Int) pure returns (Int) {
    return w(7);
}
function mul(Int, Int) pure returns (Int) {
    return w(8);
}
function div(Int, Int) pure returns (Int) {
    return w(9);
}
function mod(Int, Int) pure returns (Int) {
    return w(10);
}
function eq(Int x, Int) pure returns (bool) {
    return uw(x) == 1;
}
function noteq(Int x, Int) pure returns (bool) {
    return uw(x) == 2;
}
function lt(Int x, Int) pure returns (bool) {
    return uw(x) == 3;
}
function gt(Int x, Int) pure returns (bool) {
    return uw(x) == 4;
}
function leq(Int x, Int) pure returns (bool) {
    return uw(x) == 5;
}
function geq(Int x, Int) pure returns (bool) {
    return uw(x) == 6;
}

// TODO test that side-effects are executed properly.
contract C {
    function f1() public pure returns (Int) {
        require(w(1) | w(2) == w(1));
        require(!(w(1) | w(2) == w(2)));
        return w(1) | w(2);
    }
    // TODO all the other operators
}
// ====
// compileViaYul: also
// ----
// f1()
