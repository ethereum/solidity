type Int is int16;

using {f as +} for IntError;

error IntError();

function f(Int _a, Int _b) pure returns (Int) {
    return Int.wrap(0);
}

// ----
// TypeError 5172: (39-47): Name has to refer to a user-defined value type, struct, enum or contract.
