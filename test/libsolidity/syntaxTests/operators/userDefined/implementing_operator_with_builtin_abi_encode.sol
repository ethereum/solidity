type Int is int16;

using {abi.encode as +} for Int global;

function f(Int, Int) pure returns (Int) {}

// ----
// DeclarationError 9589: (27-37): Identifier is not a function name or not unique.
