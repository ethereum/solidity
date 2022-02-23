contract C {
    function f() public payable {
		abi.encode(this.f{value: 2});
		abi.encode(this.f{gas: 2});
		abi.encode(this.f{value: 2, gas: 1});
    }
}
// ----
// TypeError 2056: (60-76): This type cannot be encoded.
// TypeError 2056: (92-106): This type cannot be encoded.
// TypeError 2056: (122-146): This type cannot be encoded.
