type Int is int128;
using {
    add as +, sub as -,
    mul as *, div as /
     } for Int;

function add(Int, Int) pure returns (Int) {
    return Int.wrap(1);
}

function sub(int128, Int) pure returns (int128) {
    return 0;
}

function mul(int128, int256) pure returns (Int) {
    return Int.wrap(2);
}

function div(Int, Int) pure returns (int256) {
    return 3;
}
// ----
// TypeError 3100: (42-45): The function "sub" cannot be bound to the type "Int" because the type cannot be implicitly converted to the first argument of the function ("int128").
// TypeError 3100: (56-59): The function "mul" cannot be bound to the type "Int" because the type cannot be implicitly converted to the first argument of the function ("int128").
// TypeError 7743: (66-69): The function "div" needs to return exactly one value of type Int to be used for the operator /.
