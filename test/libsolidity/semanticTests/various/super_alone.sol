contract A {
    function f() public {
        super;
    }
}

// ====
// compileToEwasm: also
// ----
// f() ->
