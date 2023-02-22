type Int is int;

using {add as +} for Int global;
using {add2 as +} for Int;
using {unsub as -} for Int global;
using {unsub2 as -} for Int;

function add(Int, Int) pure returns (Int) {}
function add2(Int, Int) pure returns (Int) {}
function unsub(Int) pure returns (Int) {}
function unsub2(Int) pure returns (Int) {}

function testBinary() pure returns (Int) {
    return Int.wrap(1) + Int.wrap(2);
}

function testUnary() pure returns (Int) {
    return -Int.wrap(2);
}
// ----
// TypeError 4705: (25-28): User-defined binary operator + has more than one definition matching the operand type visible in the current scope.
// TypeError 3320: (58-62): Operators can only be defined in a global 'using for' directive.
// TypeError 4705: (58-62): User-defined binary operator + has more than one definition matching the operand type visible in the current scope.
// TypeError 4705: (85-90): User-defined unary operator - has more than one definition matching the operand type visible in the current scope.
// TypeError 3320: (120-126): Operators can only be defined in a global 'using for' directive.
// TypeError 4705: (120-126): User-defined unary operator - has more than one definition matching the operand type visible in the current scope.
