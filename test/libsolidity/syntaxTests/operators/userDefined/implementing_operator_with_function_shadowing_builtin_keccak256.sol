type Int is int16;

using {keccak256 as +} for Int;

function keccak256(Int, Int) pure returns (Int) {
    return Int.wrap(0);
}
// ----
// Warning 2319: (53-128): This declaration shadows a builtin symbol.
