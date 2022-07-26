type Int is uint128;

using {add as +, sub as +} for Int;

function add(Int, Int) returns (Int) {
    return Int.wrap(0);
}

function sub(Int, Int) returns (Int) {
    return Int.wrap(0);
}

function test() {
    Int.wrap(0) + Int.wrap(1);
}
// ----
// TypeError 2271: (213-238): Operator + not compatible with types Int and Int. Multiple user-defined functions provided for this operator.
