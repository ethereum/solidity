type Int is int;

using {add as +} for Int global;
using {another_add as +} for Int;

function add(Int, Int) pure returns (Int) {}
function another_add(Int, Int) pure returns (Int) {}

function test() pure returns (Int) {
    return Int.wrap(1) + Int.wrap(2);
}
// ----
// TypeError 5583: (233-258): User-defined binary operator + has more than one definition matching the operand types visible in the current scope.
