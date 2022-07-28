type Int is int;

using {unsub as -} for Int global;
using {another_unsub as -} for Int;

function unsub(Int) pure returns (Int) {}
function another_unsub(Int) pure returns (Int) {}

function test() pure returns (Int) {
    return -Int.wrap(2);
}
// ----
// TypeError 4705: (231-243): User-defined unary operator - has more than one definition matching the operand type visible in the current scope.
