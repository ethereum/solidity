type B is bool;
using {bitnot as ~, bitor as |} for B global;

// NOTE: There are no literals of type B so these cannot be actually used on anything
function bitor(B x, B y) pure suffix returns (B) {
    return B.wrap(B.unwrap(x) || B.unwrap(y));
}

function bitnot(B x) pure suffix returns (B) {
    return B.wrap(!B.unwrap(x));
}

contract C {
    function testBinary() pure public returns (B) {
        return B.wrap(true) | B.wrap(false);
    }

    function testUnary() pure public returns (B) {
        return ~B.wrap(true);
    }
}
// ----
// testBinary() -> true
// testUnary() -> false
