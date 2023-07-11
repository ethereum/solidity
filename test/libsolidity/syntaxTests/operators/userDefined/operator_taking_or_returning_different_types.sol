type Int is int128;

using {
    add as +,
    sub as -,
    mul as *,
    div as /
} for Int global;

function add(Int, int128) pure returns (Int) {}
function sub(int128, Int) pure returns (int128) {}
function mul(int128, int256) pure returns (Int) {}
function div(Int, Int) pure returns (int256) {}
// ----
// TypeError 1884: (115-128): Wrong parameters in operator definition. The function "add" needs to have two parameters of type Int and the same data location to be used for the operator +.
// TypeError 1884: (163-176): Wrong parameters in operator definition. The function "sub" needs to have one or two parameters of type Int and the same data location to be used for the operator -.
// TypeError 7743: (190-198): Wrong return parameters in operator definition. The function "sub" needs to return exactly one value of type Int to be used for the operator -.
// TypeError 1884: (214-230): Wrong parameters in operator definition. The function "mul" needs to have two parameters of type Int and the same data location to be used for the operator *.
// TypeError 7743: (244-249): Wrong return parameters in operator definition. The function "mul" needs to return a value of the same type and data location as its parameters to be used for the operator *.
// TypeError 7743: (289-297): Wrong return parameters in operator definition. The function "div" needs to return exactly one value of type Int to be used for the operator /.
