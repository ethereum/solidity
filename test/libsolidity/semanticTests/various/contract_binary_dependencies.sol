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
// ----
// constructor() ->
// gas irOptimized: 100415
