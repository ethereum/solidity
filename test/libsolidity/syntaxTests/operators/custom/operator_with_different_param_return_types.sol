type Int is int128;

struct S {
    int128 i;
}

using {
    add as +, sub as -,
    mul as *, div as /
} for Int;

using {
    bitor as |
} for S;

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

function bitor(S storage, S storage) pure returns (S memory) {
    return S(1);
}


// ----
// TypeError 3100: (71-74): The function "sub" cannot be bound to the type "Int" because the type cannot be implicitly converted to the first argument of the function ("int128").
// TypeError 3100: (85-88): The function "mul" cannot be bound to the type "Int" because the type cannot be implicitly converted to the first argument of the function ("int128").
// TypeError 7743: (95-98): The function "div" needs to return exactly one value of type Int to be used for the operator /.
// TypeError 7743: (128-133): The function "bitor" needs to return exactly one value of type S to be used for the operator |.
