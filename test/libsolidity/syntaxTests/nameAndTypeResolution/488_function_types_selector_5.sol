contract C {
    function h() pure external {
    }
    function f() pure external returns (bytes4) {
        return this.h.selector;
    }
}
// ----
