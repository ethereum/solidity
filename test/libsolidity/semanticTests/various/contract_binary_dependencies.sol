contract A {
    function f() public {
        new B();
    }
}


contract B {
    function f() public {}
}


contract C {
    function f() public {
        new B();
    }
}

// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// constructor() ->
