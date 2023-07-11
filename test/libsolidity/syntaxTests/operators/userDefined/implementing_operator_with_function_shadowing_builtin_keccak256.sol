type Int is int16;

using {keccak256 as +} for Int global;

function keccak256(Int, Int) pure returns (Int) {
    return Int.wrap(0);
}
// ----
// Warning 2319: (60-135): This declaration shadows a builtin symbol.
