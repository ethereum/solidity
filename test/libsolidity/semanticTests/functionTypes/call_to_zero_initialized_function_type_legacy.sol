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
        this.store((new Other()).addTwo);
        // This call panics
        return ev(x);
    }
}
// ====
// compileViaYul: false
// targetContract: C
// ----
// t() -> FAILURE, hex"4e487b71", 0x51
// gas legacy: 77534
// gas legacy code: 69600
// gas legacyOptimized: 76677
// gas legacyOptimized code: 28600
