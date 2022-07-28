type Int is int;

function add(Int, Int) pure returns (Int) {}

interface I {
    using {add as +} for Int;
}
// ----
// SyntaxError 9088: (82-107): The "using for" directive is not allowed inside interfaces.
// TypeError 3320: (89-92): Operators can only be defined in a global 'using for' directive.
