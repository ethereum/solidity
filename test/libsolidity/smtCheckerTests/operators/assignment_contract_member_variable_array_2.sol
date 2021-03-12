pragma experimental SMTChecker;
contract A {
    int[] a;
    function f() public {
        A.a[0] = 2;
    }
}

