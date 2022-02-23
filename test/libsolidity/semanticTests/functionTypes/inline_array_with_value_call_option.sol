contract C {
    function f() external payable returns (uint) { assert(msg.value > 0); return 1; }
    function g() external payable returns (uint) { assert(msg.value > 0); return 2; }

    function h() public payable returns (uint) {
        return [this.f, this.g][0]{value: 1}();
    }
}
// ====
// compileViaYul: also
// ----
// h(), 1 ether -> 1
