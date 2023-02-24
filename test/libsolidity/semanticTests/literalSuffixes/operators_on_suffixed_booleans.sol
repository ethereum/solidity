type Bool is bool;
using {bitor as |, bitand as &, bitnot as ~} for Bool global;

function bitor(Bool x, Bool y) pure returns (Bool) {
    return Bool.wrap(Bool.unwrap(x) || Bool.unwrap(y));
}

function bitand(Bool x, Bool y) pure returns (Bool) {
    return Bool.wrap(Bool.unwrap(x) && Bool.unwrap(y));
}

function bitnot(Bool x) pure returns (Bool) {
    return Bool.wrap(!Bool.unwrap(x));
}

function b(bool x) pure suffix returns (Bool) {
    return Bool.wrap(x);
}

contract C {
    function test() public pure returns (Bool) {
        return ~ ~ ~(false b | true b & ~false b) & true b;
    }
}
// ----
// test() -> false
