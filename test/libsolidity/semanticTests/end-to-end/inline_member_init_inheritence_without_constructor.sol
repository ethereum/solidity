
		contract Base {
			uint m_base = 5;
			function getBMember() public returns (uint i) { return m_base; }
		}
		contract Derived is Base {
			uint m_derived = 6;
			function getDMember() public returns (uint i) { return m_derived; }
		}
	
// ----
// getBMember() -> 5
// getDMember() -> 6

