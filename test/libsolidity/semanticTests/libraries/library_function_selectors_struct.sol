pragma experimental ABIEncoderV2;
library L {
    struct S { uint256 a; }
    function f(S storage s) external returns (uint) { return s.a; }
    function g(S memory m) public returns (uint) { return m.a; }
}
contract C {
    L.S s;
    constructor() public { s.a = 42; }

    function f() public returns (bool, bool, uint256) {
		uint256 s_ptr;
		assembly { s_ptr := s_slot }
		(bool success, bytes memory data) = address(L).delegatecall(abi.encodeWithSelector(L.f.selector, s_ptr));
		return (L.f.selector == bytes4(keccak256("f(L.S storage)")), success, abi.decode(data, (uint256)));
    }
    function g() public returns (bool, bool, uint256) {
		(bool success, bytes memory data) = address(L).delegatecall(abi.encodeWithSelector(L.g.selector, L.S(23)));
		return (L.g.selector == bytes4(keccak256("g(L.S)")), success, abi.decode(data, (uint256)));
    }
}
// ====
// EVMVersion: >homestead
// ----
// library: L
// f() -> true, true, 42
// g() -> true, true, 23
