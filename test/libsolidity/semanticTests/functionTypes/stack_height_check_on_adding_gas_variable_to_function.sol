contract C {
    function g() external {}
    function h() external payable {}
    function test_function() external returns (bool){
        assert (
            this.g.address == this.g.address &&
            this.g{gas: 42}.address == this.g.address &&
            this.g{gas: 42}.selector == this.g.selector
        );
        assert (
            this.h.address == this.h.address &&
            this.h{gas: 42}.address == this.h.address &&
            this.h{gas: 42}.selector == this.h.selector
        );
        assert (
            this.h{gas: 42, value: 5}.address == this.h.address &&
            this.h{gas: 42, value: 5}.selector == this.h.selector
        );
        return true;
    }
}
// ====
// compileViaYul: also
// ----
// test_function() -> true
