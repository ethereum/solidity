type Int is int;
using {add as +} for Int;
using {unsub as -} for Int;

function add(Int, Int) pure returns (Int) {}
function unsub(Int) pure returns (Int) {}

function f() pure {
    Int.wrap(0) + Int.wrap(0);
    -Int.wrap(0);
}
// ----
// TypeError 3320: (24-27): Operators can only be defined in a global 'using for' directive.
// TypeError 3320: (50-55): Operators can only be defined in a global 'using for' directive.
