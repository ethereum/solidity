pragma abicoder v2;

struct S {
    int value;
}

using {add as +, mul as *, unsub as -} for S;

function add(S calldata a, S calldata) returns (S calldata) {
    return a;
}

function mul(S calldata, S calldata b) returns (S calldata) {
    return b;
}

function unsub(S calldata a) returns (S calldata) {
    return a;
}

contract C {
    function testAdd(S calldata a, S calldata b) public returns (int) {
        return (a + b).value;
    }

    function testMul(S calldata a, S calldata b) public returns (int) {
        return (a * b).value;
    }

    function testUnsub(S calldata a) public returns (int) {
        return (-a).value;
    }
}
// ----
// testAdd((int256),(int256)): 3, 7 -> 3
// testMul((int256),(int256)): 3, 7 -> 7
// testUnsub((int256)): 3 -> 3
