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
// compileToEwasm: also
// compileViaYul: also
// ----
// constructor() ->
// gas irOptimized: 102813
