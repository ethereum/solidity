// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.0;

type Price is uint128;
enum Color { Red, Green, Blue }
struct Point { uint8 x; uint8 y; bytes4 salt; }
struct Line { Point from; Point to; string label; }
interface I { function f() external; }
library L { function id(uint256 a) internal pure returns (uint256) { return a; } }

contract A1 {
	uint128 stored;
	bool enabled;
	int256 signed;
	address owner;
	address payable sink;
	bytes32 hash;
	bytes blob;
	string label;
	Color color;
	Price price;
	Point point;
	Line line;
	uint16[8] packed;
	uint256[] values;
	uint256[2][] grid;
	mapping(address => uint256) balances;
	mapping(address => mapping(uint256 => Line)) lines;
	mapping(string => bool) named;
	I other;
	A1 self;
	function (uint256) internal pure returns (uint256) internalFunction;
	function (uint256) external returns (bool) externalFunction;
	uint256 transient temporary;
	uint256 constant CONSTANT = 1;

	function a(uint x) public pure {
		assert(x > 0);
	}

	function b(Point memory p, uint256[] calldata xs) public pure returns (Line memory, bytes memory) {
		assert(p.x > 0 && xs.length > 0);
	}
}

contract A2 {
	function a(uint x) public pure {
		assert(x > 0);
	}
}
