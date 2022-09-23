using {add as +, unsub as -} for S;

struct S {
    uint x;
}

function add(S memory _a, S memory) returns (S memory) {
    return _a;
}

function unsub(S memory _a) returns (S memory) {
    return _a;
}

contract C {
    S s;

    function test() public {
        S storage sTmp;
        S memory tmp;
        s + s;
        tmp + true;
        true + s;
        -sTmp;
        -s;
        -true;
    }
}

// ----
// TypeError 2271: (311-316): Built-in binary operator + cannot be applied to types struct S storage ref and struct S storage ref. No matching user-defined operator found.
// TypeError 5653: (326-336): User defined binary operator + not compatible with types struct S memory and bool.
// TypeError 2271: (346-354): Built-in binary operator + cannot be applied to types bool and struct S storage ref.
// TypeError 4907: (364-369): Built-in unary operator - cannot be applied to type struct S storage pointer. No matching user-defined operator found.
// TypeError 4907: (379-381): Built-in unary operator - cannot be applied to type struct S storage ref. No matching user-defined operator found.
// TypeError 4907: (391-396): Built-in unary operator - cannot be applied to type bool.
