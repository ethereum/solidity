contract D {
    uint256 x;

    constructor() {
        x = 7;
    }

    function f() public view returns (uint256) {
        return x;
    }
}


contract C {
    function test() public returns (uint256) {
        bytes memory c = type(D).creationCode;
        D d;
        assembly {
            d := create(0, add(c, 0x20), mload(c))
        }
        return d.f();
    }
}

// ----
// test() -> 7
// gas legacy: 100849
