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
// ----
// constructor() ->
// gas irOptimized: 56579
// gas irOptimized code: 39000
