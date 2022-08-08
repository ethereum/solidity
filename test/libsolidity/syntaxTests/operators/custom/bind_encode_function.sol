type Int is int16;

using {abi.encode as +} for Int;

function f(Int, Int) pure returns (Int) {
    return Int.wrap(0);
}

// ----
// DeclarationError 7920: (27-37): Identifier not found or not unique.
