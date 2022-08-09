type Int is int;

using {
    f as ~
} for Int;

function f() returns (Int) {
    return Int.wrap(0);
}

// ----
// TypeError 4731: (30-31): The function "f" does not have any parameters, and therefore cannot be bound to the type "Int".
