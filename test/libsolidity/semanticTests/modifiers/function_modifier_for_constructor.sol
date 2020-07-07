contract A {
    uint256 data;

    constructor() mod1 {
        data |= 2;
    }

    modifier mod1 virtual {
        data |= 1;
        _;
    }

    function getData() public returns (uint256 r) {
        return data;
    }
}


contract C is A {
    modifier mod1 override {
        data |= 4;
        _;
    }
}

// ----
// getData() -> 6
