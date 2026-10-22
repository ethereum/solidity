// Used to cause ICE in TypeChecker::visit(UnaryOperation) when a function
// body referenced an operator that had multiple matching `using for`
// definitions before the directive's own duplicate check (error 4705)
// had a chance to run.
type T is int256;
function neg1(T a) pure returns (T) { return -a; }
function neg2(T a) pure returns (T) { return a; }
using {neg1 as -, neg2 as -} for T global;
// ----
// TypeError 4705: (262-266): User-defined unary operator - has more than one definition matching the operand type visible in the current scope.
// TypeError 4705: (273-277): User-defined unary operator - has more than one definition matching the operand type visible in the current scope.
