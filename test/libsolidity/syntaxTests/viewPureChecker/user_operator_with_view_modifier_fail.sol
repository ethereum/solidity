type Int is uint8;

using {
    add as +
} for Int;


function f_view() view {}

function add(Int, Int) view returns (Int) {
    f_view();
    return Int.wrap(0);
}

function f() view {
    Int.wrap(0) + Int.wrap(0);
}

function g() {
    Int.wrap(0) + Int.wrap(0);
}

function h() pure {
    Int.wrap(0) + Int.wrap(0);
}


// ----
// Warning 2018: (220-267): Function state mutability can be restricted to view
// TypeError 2527: (293-318): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
