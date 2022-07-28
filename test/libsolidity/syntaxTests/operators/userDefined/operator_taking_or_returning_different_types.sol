type Int is int128;

using {
    add as +,
    sub as -,
    mul as *,
    div as /
} for Int;

function add(Int, int128) pure returns (Int) {}
function sub(int128, Int) pure returns (int128) {}
function mul(int128, int256) pure returns (Int) {}
function div(Int, Int) pure returns (int256) {}
// ----
// TypeError 1884: (108-121): Wrong parameters in operator definition. The function "add" needs to have two parameters of type Int and the same data location to be used for the operator +.
// TypeError 1884: (156-169): Wrong parameters in operator definition. The function "sub" needs to have one or two parameters of type Int and the same data location to be used for the operator -.
// TypeError 7743: (183-191): Wrong return parameters in operator definition. The function "sub" needs to return exactly one value of type Int to be used for the operator -.
// TypeError 1884: (207-223): Wrong parameters in operator definition. The function "mul" needs to have two parameters of type Int and the same data location to be used for the operator *.
// TypeError 7743: (237-242): Wrong return parameters in operator definition. The function "mul" needs to return a value of the same type and data location as its parameters to be used for the operator *.
// TypeError 7743: (282-290): Wrong return parameters in operator definition. The function "div" needs to return exactly one value of type Int to be used for the operator /.
