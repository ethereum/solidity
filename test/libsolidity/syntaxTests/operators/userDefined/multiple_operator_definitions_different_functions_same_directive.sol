type Int is uint128;

using {add as +, sub as +} for Int global;

function add(Int, Int) pure returns (Int) {}
function sub(Int, Int) pure returns (Int) {}

function test() {
    Int.wrap(0) + Int.wrap(1);
}
// ----
// TypeError 4705: (29-32): User-defined binary operator + has more than one definition matching the operand type visible in the current scope.
// TypeError 4705: (39-42): User-defined binary operator + has more than one definition matching the operand type visible in the current scope.
