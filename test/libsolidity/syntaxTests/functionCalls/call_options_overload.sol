contract C {
    function f(uint x) external payable { }
    function f(uint x, uint y) external payable { }
    function call() internal {
        this.f{value: 10}(2);
    }
}
// ----
// TypeError: (148-154): Member "f" not unique after argument-dependent lookup in contract C.
