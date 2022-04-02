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
// TypeError 2311: (14-26='address view'): Address types can only be payable or non-payable.
// TypeError 2311: (33-45='address pure'): Address types can only be payable or non-payable.
// TypeError 2311: (52-64='address view'): Address types can only be payable or non-payable.
// TypeError 2311: (89-101='address view'): Address types can only be payable or non-payable.
// TypeError 2311: (138-150='address view'): Address types can only be payable or non-payable.
// TypeError 2311: (156-168='address pure'): Address types can only be payable or non-payable.
// TypeError 2311: (195-207='address view'): Address types can only be payable or non-payable.
// TypeError 2311: (236-248='address pure'): Address types can only be payable or non-payable.
// TypeError 2311: (300-312='address view'): Address types can only be payable or non-payable.
// TypeError 2311: (352-364='address pure'): Address types can only be payable or non-payable.
