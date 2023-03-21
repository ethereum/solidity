type B is bool;
using {bitnot as ~, bitor as |} for B global;

// NOTE: There are no literals of type B so these cannot be actually used on anything
function bitnot(B x) pure suffix returns (B) {
    return B.wrap(!B.unwrap(x));
}

function bitor(B x, B y) pure suffix returns (B) {
    return B.wrap(B.unwrap(x) || B.unwrap(y));
}

contract C {
    function testBinary() pure public returns (B) {
        B.wrap(true) | B.wrap(false);
    }

    function testUnary() pure public returns (B) {
        -B.wrap(true);
    }
}
// ----
// testUnary() -> true
// testBinary() -> false
