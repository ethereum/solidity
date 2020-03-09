library Arst {
    struct Foo {
        int256 Things;
        int256 Stuff;
    }
}


contract Tsra {
    function f() public returns (uint256) {
        Arst.Foo;
        return 1;
    }
}

// ----
// f() -> 1
