contract A {
    struct S {
        string x;
    }
}

contract B {
    struct S {
        bool y;
    }
}

contract C is A, B {
    function f() public pure {
        assert(type(A.S).typehash != type(B.S).typehash);
    }
}
// ----
// DeclarationError 9097: (72-104): Identifier already declared.
