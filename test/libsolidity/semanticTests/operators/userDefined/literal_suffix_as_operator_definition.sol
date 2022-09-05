type B is bool;
using {bitnot as ~} for B global;

// NOTE: There are no literals of type B so these cannot be actually used on anything
function bitnot(B x) pure suffix returns (B) {
    return B.wrap(!B.unwrap(x));
}

contract C {
    function testUnary() pure public returns (B) {
        return ~B.wrap(true);
    }
}
// ----
// testUnary() -> false
