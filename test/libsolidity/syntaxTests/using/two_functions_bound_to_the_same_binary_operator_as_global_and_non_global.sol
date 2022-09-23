==== Source: s1.sol ====
type Int is int;

using {add as +} for Int global;
using {another_add as +} for Int;

function add(Int, Int) pure returns (Int) {
    return Int.wrap(0);
}

function another_add(Int, Int) pure returns (Int) {
    return Int.wrap(0);
}

function test() pure returns (Int) {
    return Int.wrap(1) + Int.wrap(2);
}

// ----
// TypeError 2271: (s1.sol:284-309): Built-in binary operator + cannot be applied to types Int and Int. Multiple user-defined functions provided for this operator.
