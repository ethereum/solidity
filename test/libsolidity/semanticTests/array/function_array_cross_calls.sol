contract D {
    function f(function() external returns (function() external returns (uint))[] memory x)
        public returns (function() external returns (uint)[3] memory r) {
        r[0] = x[0]();
        r[1] = x[1]();
        r[2] = x[2]();
    }
}


contract C {
    function test() public returns (uint256, uint256, uint256) {
        function() external returns (function() external returns (uint))[] memory x =
            new function() external returns (function() external returns (uint))[](10);
        for (uint256 i = 0; i < x.length; i++) x[i] = this.h;
        x[0] = this.htwo;
        function() external returns (uint)[3] memory y = (new D()).f(x);
        return (y[0](), y[1](), y[2]());
    }

    function e() public returns (uint256) {
        return 5;
    }

    function f() public returns (uint256) {
        return 6;
    }

    function g() public returns (uint256) {
        return 7;
    }

    uint256 counter;

    function h() public returns (function() external returns (uint)) {
        return counter++ == 0 ? this.f : this.g;
    }

    function htwo() public returns (function() external returns (uint)) {
        return this.e;
    }
}

// ----
// test() -> 5, 6, 7
// gas irOptimized: 261497
// gas legacy: 450936
// gas legacyOptimized: 284265
