// Used to cause ICE in TypeChecker::endVisit(BinaryOperation) when a
// function body referenced an operator that had multiple matching
// `using for` definitions before the directive's own duplicate check
// (error 4705) had a chance to run.
type T is int256;
function sub(T a, T b) pure returns (T) {}
function add(T a, T b) pure returns (T) { T c = a + b; c; }
using {sub as +, add as +} for T global;
// ----
// TypeError 4705: (267-270): User-defined binary operator + has more than one definition matching the operand type visible in the current scope.
// TypeError 4705: (277-280): User-defined binary operator + has more than one definition matching the operand type visible in the current scope.
