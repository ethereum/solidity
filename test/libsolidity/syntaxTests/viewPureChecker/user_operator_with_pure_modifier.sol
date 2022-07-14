type Int is uint8;

using {
    add as +
} for Int;

function add(Int, Int) pure returns (Int) {
    return Int.wrap(0);
}

function f() pure {
    Int.wrap(0) + Int.wrap(1);
}

function g() {
    Int.wrap(0) + Int.wrap(1);
}


// ----
// Warning 2018: (178-225): Function state mutability can be restricted to pure
