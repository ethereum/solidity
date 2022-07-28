type Int is int;

function add(Int, Int) pure returns (Int) {}

interface I {
    using {add as +} for Int;
}
// ----
// SyntaxError 9088: (82-107): The "using for" directive is not allowed inside interfaces.
