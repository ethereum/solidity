contract A {
    struct S {
        string x;
        bool[10][] y;
    }
}

contract C is A {
    function f() public pure returns(bool) {
        return type(S).typehash == keccak256("S(string x,bool[10][] y)");
    }
}
// ----
// f() -> true
