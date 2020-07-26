library Arst {
    enum Foo {Things, Stuff}
}


contract Tsra {
    function f() public returns (uint256) {
        Arst.Foo;
        return 1;
    }
}

// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// f() -> 1
