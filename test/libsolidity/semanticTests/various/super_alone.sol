contract A {
    function f() public {
        super;
    }
}

// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// f() ->
