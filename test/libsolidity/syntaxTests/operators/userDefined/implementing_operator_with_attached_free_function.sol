type Int is uint;
using {f} for Int;

Int constant v;
using {v.f as +} for Int;

function f(Int) pure returns (Int) {}
// ----
// DeclarationError 7920: (61-64): Identifier not found or not unique.
