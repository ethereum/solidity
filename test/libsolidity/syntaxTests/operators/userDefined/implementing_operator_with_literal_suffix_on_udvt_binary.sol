type B is bool;
using {bitor as |} for B global;

function bitor(B x, B y) pure suffix returns (B) {
    return B.wrap(B.unwrap(x) || B.unwrap(y));
}
// ----
// TypeError 1587: (64-74): Literal suffix function has invalid parameter types. The mantissa parameter must be an integer. The exponent parameter must be an unsigned integer.
