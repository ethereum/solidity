type Int is int16;

using {add as +} for Int;

function add(Int, Int) returns (Int) {
    return Int.wrap(0);
}

function f() {
    Int a;
    a.add(a);
}

// ----
// TypeError 9582: (143-148): Member "add" not found or not visible after argument-dependent lookup in Int.
