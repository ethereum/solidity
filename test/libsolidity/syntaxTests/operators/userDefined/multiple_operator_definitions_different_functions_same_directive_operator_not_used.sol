type Int is uint128;

// Still an error, even if the operator is not actually used
using {add1 as +, add2 as +} for Int global;
using {unsub1 as -, unsub2 as -} for Int global;

function add1(Int, Int) pure returns (Int) {}
function add2(Int, Int) pure returns (Int) {}
function unsub1(Int) pure returns (Int) {}
function unsub2(Int) pure returns (Int) {}
// ----
// TypeError 4705: (90-94): User-defined binary operator + has more than one definition matching the operand type visible in the current scope.
// TypeError 4705: (101-105): User-defined binary operator + has more than one definition matching the operand type visible in the current scope.
// TypeError 4705: (135-141): User-defined unary operator - has more than one definition matching the operand type visible in the current scope.
// TypeError 4705: (148-154): User-defined unary operator - has more than one definition matching the operand type visible in the current scope.
