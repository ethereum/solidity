contract C {
    uint256 public x;

    function f(uint256 y) public payable {
        x = y;
    }

    function g(uint256 y) external {
        x = y;
    }

    function h() public {
        this.g(12);
    }
}


contract D {
    C c = new C();

    function f() public payable returns (uint256) {
        c.g(3);
        return c.x();
    }

    function g() public returns (uint256) {
        c.g(8);
        return c.x();
    }

    function h() public returns (uint256) {
        c.h();
        return c.x();
    }
}

// ====
// compileViaYul: also
// ----
// f() -> 3
// g() -> 8
// h() -> 12
