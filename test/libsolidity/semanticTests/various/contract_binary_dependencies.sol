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
// gas ir: 204030
// gas irOptimized: 124731
// gas legacy: 133491
// gas legacyOptimized: 133491
