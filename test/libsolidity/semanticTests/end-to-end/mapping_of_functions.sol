contract Flow {
    bool public success;

    mapping(address => function() internal) stages;

    function stage0() internal {
        stages[msg.sender] = stage1;
    }

    function stage1() internal {
        stages[msg.sender] = stage2;
    }

    function stage2() internal {
        success = true;
    }

    constructor() public {
        stages[msg.sender] = stage0;
    }

    function f() public returns (uint256) {
        stages[msg.sender]();
        return 7;
    }
}

// ----
// success() -> false
// f() -> 7
// f() -> 7
// success() -> false
// f() -> 7
// success() -> true
