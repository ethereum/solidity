contract C {
	address view m_a;
	address pure m_b;
	address view[] m_c;
	mapping(uint => address view) m_d;
	function f() public pure {
		address view a;
		address pure b;
		a; b;
	}
	function g(address view) public pure {}
	function h(address pure) public pure {}
	function i() public pure returns (address view) {}
	function j() public pure returns (address pure) {}
}
// ----
// TypeError: (14-26): Address types can only be payable or non-payable.
// TypeError: (33-45): Address types can only be payable or non-payable.
// TypeError: (52-64): Address types can only be payable or non-payable.
// TypeError: (89-101): Address types can only be payable or non-payable.
// TypeError: (195-207): Address types can only be payable or non-payable.
// TypeError: (236-248): Address types can only be payable or non-payable.
// TypeError: (300-312): Address types can only be payable or non-payable.
// TypeError: (352-364): Address types can only be payable or non-payable.
// TypeError: (138-150): Address types can only be payable or non-payable.
// TypeError: (156-168): Address types can only be payable or non-payable.
