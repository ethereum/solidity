
		contract test {
			constructor() public {
				m_b = 6;
				m_c = 8;
			}
			uint m_a = 5;
			uint m_b;
			uint m_c = 7;
			function get() public returns (uint a, uint b, uint c){
				a = m_a;
				b = m_b;
				c = m_c;
			}
		}
	
// ----
// get() -> 5, 6, 8

