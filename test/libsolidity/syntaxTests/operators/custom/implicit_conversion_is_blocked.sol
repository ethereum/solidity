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
        S memory sTmp;
        s + s;
        sTmp + true;
        true + s;
        -sTmp;
        -s;
        -true;
    }
}

// ----
// TypeError 2271: (288-293): Operator + not compatible with types struct S storage ref and struct S storage ref. No matching user-defined operator found.
// TypeError 5653: (303-314): User defined operator + not compatible with types struct S memory and bool.
// TypeError 2271: (324-332): Operator + not compatible with types bool and struct S storage ref.
// TypeError 4907: (357-359): Unary operator - cannot be applied to type struct S storage ref. No matching user-defined operator found.
// TypeError 4907: (369-374): Unary operator - cannot be applied to type bool.
