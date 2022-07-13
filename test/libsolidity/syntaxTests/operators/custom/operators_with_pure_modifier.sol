type Int is uint8;

using {
    add as +,
    sub as -,
    mul as *
} for Int;

function f_view() view {}

function add(Int, Int) pure returns (Int) {
    return Int.wrap(0);
}

function sub(Int, Int) returns (Int) {
    return Int.wrap(0);
}

function mul(Int, Int) pure returns (Int) {
    f_view();
    return Int.wrap(0);
}

// ----
// Warning 2018: (179-243): Function state mutability can be restricted to pure
// TypeError 2527: (293-301): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
