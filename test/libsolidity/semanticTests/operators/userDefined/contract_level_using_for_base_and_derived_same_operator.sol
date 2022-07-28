type Int is int128;

function addA(Int, Int) pure returns (Int) {
    return Int.wrap(1);
}

function addB(Int, Int) pure returns (Int) {
    return Int.wrap(3);
}

function addC(Int, Int) pure returns (Int) {
    return Int.wrap(7);
}

contract A {
    using {addA as +} for Int;

    function testA() pure public returns (Int) {
        return Int.wrap(0) + Int.wrap(0);
    }
}

contract B is A {
    using {addB as +} for Int;

    function testB() pure public returns (Int) {
        return Int.wrap(0) + Int.wrap(0);
    }
}

contract C is A, B {
    using {addC as +} for Int;

    function testC() pure public returns (Int) {
        return Int.wrap(0) + Int.wrap(0);
    }
}
// ----
// testA() -> 1
// testB() -> 3
// testC() -> 7
