type Length is uint;

function km(uint meters) pure returns (Length) {
    return Length.wrap(meters * 1000);
}

struct Float {
    uint mantissa;
    int exponent;
}

function f(uint mantissa, int exponent) pure returns (Float memory) {
    return Float(mantissa, exponent);
}

contract C {
    Length public length = 1000 km;
    Float public factor = 1.23 f;
}
// ----
// length() -> 1000000
// factor() -> 0x7b, 2
