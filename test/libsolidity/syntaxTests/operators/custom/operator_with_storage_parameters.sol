using {
    add as +,
    sub as -,
    mul as *,
    div as /,
    mod as %,
    unsub as -,
    bitnot as ~
} for S;

struct S {
    uint x;
}

function add(S storage a, S storage) pure returns (S storage) {
    return a;
}

function sub(S storage a, uint) pure returns (S storage) {
    return a;
}

function mul(S storage a) pure returns (S storage) {
    return a;
}

function div(S storage a, S storage) pure returns (uint) {
    return 0;
}

function mod(S storage a, S storage) pure {
}

function unsub(S storage a) pure {
}

function bitnot(S storage a, S storage) pure returns (S storage) {
    return a;
}

contract C {
    S a;
    S b;

    function test() public view {
        a + b;
        a - b;
        a * b;
        a / b;
        a % b;
        -a;
        ~a;
    }
}
// ----
// TypeError 7617: (26-29): The function "sub" needs to have one or two parameters of type S and the same data location to be used for the operator -.
// TypeError 1884: (40-43): The function "mul" needs to have two parameters of type S and the same data location to be used for the operator *.
// TypeError 7743: (54-57): The function "div" needs to return exactly one value of type S to be used for the operator /.
// TypeError 7743: (68-71): The function "mod" needs to return exactly one value of type S to be used for the operator %.
// TypeError 7743: (82-87): The function "unsub" needs to return exactly one value of type S to be used for the operator -.
// TypeError 1147: (98-104): The function "bitnot" needs to have exactly one parameter of type S to be used for the operator ~.
// TypeError 2271: (722-727): Operator * not compatible with types struct S storage ref and struct S storage ref. No matching user-defined operator found.
// TypeError 4907: (779-781): Unary operator ~ cannot be applied to type struct S storage ref. No matching user-defined operator found.
