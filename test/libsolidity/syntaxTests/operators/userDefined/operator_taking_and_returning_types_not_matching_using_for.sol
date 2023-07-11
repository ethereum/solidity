type Int is int256;

using {
    add as +,
    sub as -,
    div as /
} for Int global;

function add(Int) pure returns (Int) {}
function sub(Int, Int, Int) pure returns (Int) {}
function div(int256, int256) pure returns (Int) {}

function f() pure {
    Int.wrap(0) + Int.wrap(1);
    Int.wrap(0) - Int.wrap(0);
    Int.wrap(0) / Int.wrap(0);
}

// ----
// TypeError 1884: (101-106): Wrong parameters in operator definition. The function "add" needs to have two parameters of type Int and the same data location to be used for the operator +.
// TypeError 1884: (141-156): Wrong parameters in operator definition. The function "sub" needs to have one or two parameters of type Int and the same data location to be used for the operator -.
// TypeError 1884: (191-207): Wrong parameters in operator definition. The function "div" needs to have one or two parameters of type Int and the same data location to be used for the operator /.
// TypeError 7743: (221-226): Wrong return parameters in operator definition. The function "div" needs to return a value of the same type and data location as its parameters to be used for the operator /.
// TypeError 2271: (255-280): Built-in binary operator + cannot be applied to types Int and Int. No matching user-defined operator found.
// TypeError 2271: (286-311): Built-in binary operator - cannot be applied to types Int and Int. No matching user-defined operator found.
// TypeError 2271: (317-342): Built-in binary operator / cannot be applied to types Int and Int. No matching user-defined operator found.
