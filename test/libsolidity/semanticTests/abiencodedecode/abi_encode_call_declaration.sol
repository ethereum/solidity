pragma abicoder v2;

contract X {
    // no "returns" on purpose
    function a(uint) public pure {}
    function b(uint) external pure {}
}

contract Base {
    function a(uint x) external pure returns (uint) { return x + 1; }
}

contract C is Base {
	function test() public view returns (uint r) {
        bool success;
        bytes memory result;
        (success, result) = address(this).staticcall(abi.encodeCall(X.a, 1));
        require(success && result.length == 32);
        r += abi.decode(result, (uint));
        require(r == 2);

        (success, result) = address(this).staticcall(abi.encodeCall(X.b, 10));
        require(success && result.length == 32);
        r += abi.decode(result, (uint));
        require(r == 13);

        (success, result) = address(this).staticcall(abi.encodeCall(Base.a, 100));
        require(success && result.length == 32);
        r += abi.decode(result, (uint));
        require(r == 114);

        (success, result) = address(this).staticcall(abi.encodeCall(this.a, 1000));
        require(success && result.length == 32);
        r += abi.decode(result, (uint));
        require(r == 1115);

        (success, result) = address(this).staticcall(abi.encodeCall(C.b, 10000));
        require(success && result.length == 32);
        r += abi.decode(result, (uint));
        require(r == 11116);

        return r;
	}
    function b(uint x) external view returns (uint) {
        return this.a(x);
    }

}
// ====
// EVMVersion: >=byzantium
// ----
// test() -> 11116
