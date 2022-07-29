type Int is int256;

using {
    add as +, sub as -, div as /
} for Int;

function add(Int) pure returns (Int) {
    return Int.wrap(0);
}

function sub(Int, Int, Int) pure returns (Int) {
    return Int.wrap(1);
}

function div(int256, int256) pure returns (Int) {
    return Int.wrap(2);
}

function f() pure {
    Int.wrap(0) + Int.wrap(1);
    Int.wrap(0) - Int.wrap(0);
    Int.wrap(0) / Int.wrap(0);
    Int.wrap(0) * Int.wrap(0);
}

// ----
// TypeError 1884: (33-36): The function "add" needs to have two parameters of equal type to be used for the operator +.
// TypeError 8112: (43-46): The function "sub" needs to have one or two parameters to be used for the operator -.
// TypeError 3100: (53-56): The function "div" cannot be bound to the type "Int" because the type cannot be implicitly converted to the first argument of the function ("int256").
// TypeError 2271: (317-342): Operator + not compatible with types Int and Int. No matching user-defined operator found.
// TypeError 2271: (348-373): Operator - not compatible with types Int and Int. No matching user-defined operator found.
// TypeError 2271: (379-404): Operator / not compatible with types Int and Int. No matching user-defined operator found.
// TypeError 2271: (410-435): Operator * not compatible with types Int and Int. No matching user-defined operator found.
