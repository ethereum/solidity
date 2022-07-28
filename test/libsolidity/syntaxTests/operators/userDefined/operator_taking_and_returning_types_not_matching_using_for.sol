type Int is int256;

using {
    add as +,
    sub as -,
    div as /
} for Int;

function add(Int) pure returns (Int) {}
function sub(Int, Int, Int) pure returns (Int) {}
function div(int256, int256) pure returns (Int) {}

function f() pure {
    Int.wrap(0) + Int.wrap(1);
    Int.wrap(0) - Int.wrap(0);
    Int.wrap(0) / Int.wrap(0);
}

// ----
// TypeError 1884: (94-99): Wrong parameters in operator definition. The function "add" needs to have two parameters of type Int and the same data location to be used for the operator +.
// TypeError 1884: (134-149): Wrong parameters in operator definition. The function "sub" needs to have one or two parameters of type Int and the same data location to be used for the operator -.
// TypeError 1884: (184-200): Wrong parameters in operator definition. The function "div" needs to have one or two parameters of type Int and the same data location to be used for the operator /.
// TypeError 7743: (214-219): Wrong return parameters in operator definition. The function "div" needs to return a value of the same type and data location as its parameters to be used for the operator /.
// TypeError 2271: (248-273): Built-in binary operator + cannot be applied to types Int and Int. No matching user-defined operator found.
// TypeError 2271: (279-304): Built-in binary operator - cannot be applied to types Int and Int. No matching user-defined operator found.
// TypeError 2271: (310-335): Built-in binary operator / cannot be applied to types Int and Int. No matching user-defined operator found.
