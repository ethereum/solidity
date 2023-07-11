type Int is uint128;

using {add as +, add128 as +} for Int global;

function add(Int, Int) pure returns (Int) {}
function add128(Int, int128) pure returns (Int) {}

function test() {
    Int.wrap(0) + Int.wrap(1);
}
// ----
// TypeError 4705: (29-32): User-defined binary operator + has more than one definition matching the operand type visible in the current scope.
// TypeError 1884: (129-142): Wrong parameters in operator definition. The function "add128" needs to have two parameters of type Int and the same data location to be used for the operator +.
// TypeError 4705: (39-45): User-defined binary operator + has more than one definition matching the operand type visible in the current scope.
