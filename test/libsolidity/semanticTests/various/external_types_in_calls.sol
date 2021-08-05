contract C1 {
    C1 public bla;

    constructor(C1 x) {
        bla = x;
    }
}


contract C {
    function test() public returns (C1 x, C1 y) {
        C1 c = new C1(C1(address(9)));
        x = c.bla();
        y = this.t1(C1(address(7)));
    }

    function t1(C1 a) public returns (C1) {
        return a;
    }

    function t2() public returns (C1) {
        return C1(address(9));
    }
}

// ====
// compileViaYul: also
// ----
// test() -> 9, 7
// gas legacy: 127206
// t2() -> 9
