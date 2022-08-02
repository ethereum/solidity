type Int is int;

function add1(Int, Int) pure returns (Int) {
    return Int.wrap(1);
}

function add2(Int, Int) pure returns (Int) {
    return Int.wrap(2);
}

contract C1 {
    using {add1 as +} for Int;

    function f() public returns (Int) {
        return Int.wrap(0) + Int.wrap(0);
    }
}

contract C2 {
    using {add2 as +} for Int;

    function f() public returns (Int) {
        return Int.wrap(0) + Int.wrap(0);
    }
}

contract C {
    function test1() public returns (Int) {
        C1 c = new C1();
        return c.f();
    }

    function test2() public returns (Int) {
        C2 c = new C2();
        return c.f();
    }
}

// ----
// test1() -> 1
// test2() -> 2
