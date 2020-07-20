contract C {
    function f(uint x) external payable returns (uint) { return 1; }
    function f(uint x, uint y) external payable returns (uint) { return 2; }
    function call() public payable returns (uint v, uint x, uint y, uint z) {
        v = this.f{value: 10}(2);
        x = this.f{gas: 1000}(2, 3);
        y = this.f{gas: 1000, value: 10}(2, 3);
        z = this.f{gas: 1000}{value: 10}(2, 3);
    }
    receive() external payable {}
}
// ====
// compileViaYul: also
// ----
// (), 1 ether
// call() -> 1, 2, 2, 2
