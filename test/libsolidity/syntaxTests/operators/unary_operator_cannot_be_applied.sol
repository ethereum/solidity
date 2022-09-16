type Int is int256;

function f() pure {
    Int a = Int.wrap(0);
    -a;
    a++;
}

// ----
// TypeError 4907: (70-72): Unary operator - cannot be applied to type Int. No matching user-defined operator found.
// TypeError 9767: (78-81): Unary operator ++ cannot be applied to type Int.
