type Int is uint;
using {f} for Int;

Int constant v;
using {v.f as +} for Int global;

function f(Int) pure returns (Int) {}
// ----
// DeclarationError 9589: (61-64): Identifier is not a function name or not unique.
