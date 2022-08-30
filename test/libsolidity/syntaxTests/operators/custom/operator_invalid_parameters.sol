type Int is int256;

using {
    add as +,
    sub as -,
    div as /
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
}

// ----
// TypeError 1884: (33-36): The function "add" needs to have two parameters of type Int and the same data location to be used for the operator +.
// TypeError 8112: (47-50): The function "sub" needs to have one or two parameters of type Int and the same data location to be used for the operator -.
// TypeError 7617: (61-64): The function "div" needs to have one or two parameters of type Int and the same data location to be used for the operator /.
// TypeError 3605: (61-64): The function "div" needs to have parameters and return value of the same type to be used for the operator /.
// TypeError 2271: (325-350): Binary operator + not compatible with types Int and Int. No matching user-defined operator found.
// TypeError 2271: (356-381): Binary operator - not compatible with types Int and Int. No matching user-defined operator found.
// TypeError 2271: (387-412): Binary operator / not compatible with types Int and Int. No matching user-defined operator found.
