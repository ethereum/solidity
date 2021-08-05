contract Other {
    function addTwo(uint256 x) public returns (uint256) {
        return x + 2;
    }
}


contract C {
    function (function (uint) external returns (uint)) internal returns (uint) ev;
    function (uint) external returns (uint) x;

    function store(function(uint) external returns (uint) y) public {
        x = y;
    }

    function eval(function(uint) external returns (uint) y) public returns (uint) {
        return y(7);
    }

    function t() public returns (uint256) {
        ev = eval;
        this.store((new Other()).addTwo);
        return ev(x);
    }
}

// ====
// compileViaYul: also
// ----
// t() -> 9
// gas irOptimized: 99004
// gas legacy: 159048
// gas legacyOptimized: 108916
