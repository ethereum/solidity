contract Base {
    uint256 public x;

    function deleteX() internal {
        delete x;
    }
}

contract C is Base {
    uint256 transient t;

    function setAndClear() external {
        x = 1;
        t = 2;
        deleteX();
        delete t;
        assert(t == 0);
    }
}

// ====
// EVMVersion: >=cancun
// targetContract: C
// ----
// setAndClear() ->
// x() -> 0
