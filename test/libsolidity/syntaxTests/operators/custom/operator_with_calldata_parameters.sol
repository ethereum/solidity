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

function add(S calldata, S calldata) pure returns (S calldata r) {
    assembly {
        r := 0
    }
}

function sub(S calldata, uint) pure returns (S calldata r) {
    assembly {
        r := 0
    }
}

function mul(S calldata) pure returns (S calldata r) {
    assembly {
        r := 0
    }
}

function div(S calldata, S calldata) pure returns (uint) {
    return 0;
}

function mod(S calldata, S calldata) pure {
}

function unsub(uint) pure returns (S calldata r) {
    assembly {
        r := 0
    }
}

function bitnot(S calldata) pure {
}

function test(S calldata s) pure {
    s + s;
    s - s;
    s * s;
    s / s;
    s % s;
    -s;
    ~s;
}

// ----
// TypeError 1884: (40-43): The function "mul" needs to have two parameters of equal type to be used for the operator *.
// TypeError 7743: (54-57): The function "div" needs to return exactly one value of type S to be used for the operator /.
// TypeError 7743: (68-71): The function "mod" needs to return exactly one value of type S to be used for the operator %.
// TypeError 3100: (82-87): The function "unsub" cannot be bound to the type "struct S storage pointer" because the type cannot be implicitly converted to the first argument of the function ("uint256").
// TypeError 7743: (98-104): The function "bitnot" needs to return exactly one value of type S to be used for the operator ~.
// TypeError 5653: (747-752): User defined operator - needs to have two parameters of equal type.
// TypeError 2271: (758-763): Operator * not compatible with types struct S calldata and struct S calldata. No matching user-defined operator found.
// TypeError 3841: (769-774): User defined operator / needs to return value of type struct S calldata.
// TypeError 1208: (780-785): User defined operator % needs to return exactly one value.
// TypeError 4907: (791-793): Unary operator - cannot be applied to type struct S calldata. No matching user-defined operator found.
// TypeError 3138: (799-801): User defined operator ~ needs to return exactly one value.
