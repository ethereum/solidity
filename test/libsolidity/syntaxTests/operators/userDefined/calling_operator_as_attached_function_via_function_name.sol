type Int is int16;

using {add as +} for Int global;

function add(Int, Int) pure returns (Int) {}

function f() {
    Int a;
    a.add(a);
}
// ----
// TypeError 9582: (130-135): Member "add" not found or not visible after argument-dependent lookup in Int.
