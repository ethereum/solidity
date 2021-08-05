contract A {
    function f(uint16 input) public pure returns (uint16[5] memory arr) {
        arr[0] = input;
        arr[1] = ++input;
        arr[2] = ++input;
        arr[3] = ++input;
        arr[4] = ++input;
    }
}


contract B {
    function f() public returns (uint16[5] memory res, uint16[5] memory res2) {
        A a = new A();
        res = a.f(2);
        res2 = a.f(1000);
    }
}

// ====
// compileViaYul: also
// ----
// f() -> 2, 3, 4, 5, 6, 1000, 1001, 1002, 1003, 1004
// gas irOptimized: 121038
// gas legacy: 235269
// gas legacyOptimized: 133299
