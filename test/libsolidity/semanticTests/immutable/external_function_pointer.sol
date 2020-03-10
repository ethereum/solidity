contract D {
    function f() external view returns (uint256) {
        return 42;
    }
}
contract C {
    D d;
    function() external view returns(uint256) immutable z;
    constructor() public {
        d = new D();
        z = d.f;
    }
    function f() public view returns (uint256) {
	assert(z.address == address(d));
	assert(z.selector == D.f.selector);
        return z();
    }
}
// ----
// f() -> 42
