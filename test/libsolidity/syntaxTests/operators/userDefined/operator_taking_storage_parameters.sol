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

function add(S storage a, S storage) pure returns (S storage) {}
function sub(S storage a, uint) pure returns (S storage) {}
function mul(S storage a) pure returns (S storage) {}
function div(S storage a, S storage) pure returns (uint) {}
function mod(S storage a, S storage) pure {}
function unsub(S storage a) pure {}
function bitnot(S storage a, S storage) pure returns (S storage) {}

contract C {
    S a;
    S b;

    function test() public view {
        S storage c;
        S storage d;

        // storage ref
        a + b; // OK
        a - b;
        a * b;
        a / b;
        a % b;
        -a;
        ~a;

        // storage ptr
        c + d; // OK
        c - d;
        c * a;
        a / d;
        -c;
        ~c;
    }
}
// ----
// TypeError 1884: (223-242): Wrong parameters in operator definition. The function "sub" needs to have one or two parameters of type S and the same data location to be used for the operator -.
// TypeError 1884: (283-296): Wrong parameters in operator definition. The function "mul" needs to have two parameters of type S and the same data location to be used for the operator *.
// TypeError 7743: (375-381): Wrong return parameters in operator definition. The function "div" needs to return exactly one value of type S to be used for the operator /.
// TypeError 7743: (427-427): Wrong return parameters in operator definition. The function "mod" needs to return exactly one value of type S to be used for the operator %.
// TypeError 7743: (463-463): Wrong return parameters in operator definition. The function "unsub" needs to return exactly one value of type S to be used for the operator -.
// TypeError 1884: (481-505): Wrong parameters in operator definition. The function "bitnot" needs to have exactly one parameter of type S to be used for the operator ~.
// TypeError 2271: (711-716): Built-in binary operator * cannot be applied to types struct S storage ref and struct S storage ref. No matching user-defined operator found.
// TypeError 4907: (768-770): Built-in unary operator ~ cannot be applied to type struct S storage ref. No matching user-defined operator found.
// TypeError 2271: (840-845): Built-in binary operator * cannot be applied to types struct S storage pointer and struct S storage ref. No matching user-defined operator found.
// TypeError 4907: (882-884): Built-in unary operator ~ cannot be applied to type struct S storage pointer. No matching user-defined operator found.
