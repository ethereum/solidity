type Int is int32;

using {add as +, add as +} for Int;

function add(Int, Int) pure returns(Int) {
    return Int.wrap(0);
}

function f() pure {
    Int.wrap(0) + Int.wrap(0);
}

// ----
