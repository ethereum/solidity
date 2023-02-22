type Int is uint8;

using {add as +} for Int global;

function add(Int, Int) pure returns (Int) {}

function f() pure {
    Int.wrap(0) + Int.wrap(1);
}

function g() view {
    Int.wrap(0) + Int.wrap(1);
}

function h() {
    Int.wrap(0) + Int.wrap(1);
}
// ----
// Warning 2018: (154-206): Function state mutability can be restricted to pure
// Warning 2018: (208-255): Function state mutability can be restricted to pure
