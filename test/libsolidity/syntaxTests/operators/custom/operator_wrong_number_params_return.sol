type Int is int256;

using {
    add as +, sub as -,
    mul as *, div as /,
    gt as >, lt as <
     } for Int;

function add(Int) pure returns (Int) {
    return Int.wrap(0);
}

function sub(Int, Int, Int) pure returns (Int) {
    return Int.wrap(1);
}

function mul(Int) pure returns (Int) {
    return Int.wrap(2);
}

function div(Int x, Int y) pure {
    x = y;
}

function gt(Int, Int) pure returns (Int) {
    return Int.wrap(0);
}

function lt(Int, Int) pure returns (bool, Int) {
    return (true, Int.wrap(1));
}

// ----
// TypeError 1884: (33-36): The function "add" needs to have two parameters of equal type to be used for the operator +.
// TypeError 8112: (43-46): The function "sub" needs to have one or two parameters to be used for the operator -.
// TypeError 1884: (57-60): The function "mul" needs to have two parameters of equal type to be used for the operator *.
// TypeError 7743: (67-70): The function "div" needs to return exactly one value of type Int to be used for the operator /.
// TypeError 7995: (81-83): The function "gt" needs to return exactly one value of type bool to be used for the operator >.
// TypeError 7995: (90-92): The function "lt" needs to return exactly one value of type bool to be used for the operator <.
