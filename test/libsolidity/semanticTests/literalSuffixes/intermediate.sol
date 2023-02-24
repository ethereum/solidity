type Length is uint;

function km(uint meters) pure suffix returns (Length) {
    return Length.wrap(meters * 1000);
}

struct Float {
    uint mantissa;
    uint exponent;
}

function f(uint mantissa, uint exponent) pure suffix returns (Float memory) {
    return Float(mantissa, exponent);
}

contract C {
    Length public length = 5000 km;
    Float public factor = 1.23 f;
}
// ----
// length() -> 5000000
// factor() -> 0x7b, 2
