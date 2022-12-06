using {
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

function sub(S calldata, uint) pure returns (S calldata r) {}
function mul(S calldata) pure returns (S calldata r) {}
function div(S calldata, S calldata) pure returns (uint) {}
function mod(S calldata, S calldata) pure {}
function unsub(uint) pure returns (S calldata r) {}
function bitnot(S calldata) pure {}

function test(S calldata s) pure {
    s - s;
    s * s;
    s / s;
    s % s;
    -s;
    ~s;
}
// ----
// TypeError 1884: (144-162): Wrong parameters in operator definition. The function "sub" needs to have one or two parameters of type S and the same data location to be used for the operator -.
// TypeError 1884: (206-218): Wrong parameters in operator definition. The function "mul" needs to have two parameters of type S and the same data location to be used for the operator *.
// TypeError 7743: (300-306): Wrong return parameters in operator definition. The function "div" needs to return exactly one value of type S to be used for the operator /.
// TypeError 7743: (352-352): Wrong return parameters in operator definition. The function "mod" needs to return exactly one value of type S to be used for the operator %.
// TypeError 1884: (369-375): Wrong parameters in operator definition. The function "unsub" needs to have one or two parameters of type S and the same data location to be used for the operator -.
// TypeError 7743: (389-403): Wrong return parameters in operator definition. The function "unsub" needs to return a value of the same type and data location as its parameters to be used for the operator -.
// TypeError 7743: (440-440): Wrong return parameters in operator definition. The function "bitnot" needs to return exactly one value of type S to be used for the operator ~.
// TypeError 2271: (494-499): Built-in binary operator * cannot be applied to types struct S calldata and struct S calldata. No matching user-defined operator found.
// TypeError 4907: (527-529): Built-in unary operator - cannot be applied to type struct S calldata. No matching user-defined operator found.
