contract A1 { constructor() {} }
contract B1 is A1 {}

contract A2 { constructor() payable {} }
contract B2 is A2 {}

contract B3 {}

contract B4 { constructor() {} }

contract C {
	function createWithValue(bytes memory c, uint256 value) public payable returns (bool) {
		uint256 y = 0;
		assembly { y := create(value, add(c, 0x20), mload(c)) }
		return y != 0;
	}
	function f(uint256 value) public payable returns (bool) {
		return createWithValue(type(B1).creationCode, value);
	}
	function g(uint256 value) public payable returns (bool) {
		return createWithValue(type(B2).creationCode, value);
	}
	function h(uint256 value) public payable returns (bool) {
		return createWithValue(type(B3).creationCode, value);
	}
	function i(uint256 value) public payable returns (bool) {
		return createWithValue(type(B4).creationCode, value);
	}
}
// ====
// EVMVersion: >homestead
// compileViaYul: also
// ----
// f(uint256), 2000 ether: 0 -> true
// f(uint256), 2000 ether: 100 -> false
// g(uint256), 2000 ether: 0 -> true
// g(uint256), 2000 ether: 100 -> false
// h(uint256), 2000 ether: 0 -> true
// h(uint256), 2000 ether: 100 -> false
// i(uint256), 2000 ether: 0 -> true
// i(uint256), 2000 ether: 100 -> false
