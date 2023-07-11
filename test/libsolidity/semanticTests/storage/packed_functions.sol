contract C {
    // these should take the same slot
    function() internal returns (uint) a;
    function() external returns (uint) b;
    function() external returns (uint) c;
    function() internal returns (uint) d;
    uint8 public x;

    function set() public {
        x = 2;
        d = g;
        c = this.h;
        b = this.h;
        a = g;
    }

    function t1() public returns (uint256) {
        return a();
    }

    function t2() public returns (uint256) {
        return b();
    }

    function t3() public returns (uint256) {
        return a();
    }

    function t4() public returns (uint256) {
        return b();
    }

    function g() public returns (uint256) {
        return 7;
    }

    function h() public returns (uint256) {
        return 8;
    }
}
// ----
// set() ->
// t1() -> 7
// t2() -> 8
// t3() -> 7
// t4() -> 8
// x() -> 2
