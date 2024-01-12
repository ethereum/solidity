contract C {
    function get(uint256 addr) external view returns (uint256 x) {
        assembly {
            x := tload(addr)
        }
    }
    function set(uint256 addr, uint256 x) external {
        assembly {
            tstore(addr, x)
        }
    }
    function test() public {
        assert(this.get(0) == 0 && this.get(42) == 0);
        this.set(0, 21);
        assert(this.get(0) == 21 && this.get(42) == 0);
        this.set(42, 131);
        assert(this.get(0) == 21 && this.get(42) == 131);
    }
}
// ====
// EVMVersion: >=cancun
// ----
// test() ->
// test() ->
