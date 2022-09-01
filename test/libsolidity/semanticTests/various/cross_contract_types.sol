contract Lib {
    struct S {
        uint256 a;
        uint256 b;
    }
}


contract Test {
    function f() public returns (uint256 r) {
        Lib.S memory x = Lib.S({a: 2, b: 3});
        r = x.b;
    }
}

// ====
// compileToEwasm: also
// ----
// f() -> 3
