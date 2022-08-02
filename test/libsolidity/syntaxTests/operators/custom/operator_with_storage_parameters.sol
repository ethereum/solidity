using {add as +} for S;

struct S {
    uint x;
}

function add(S storage a, S storage) pure returns (S storage) {
    return a;
}

contract C {
    S a;
    S b;

    function test() public view {
        a + b;
    }
}
