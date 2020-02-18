
		contract BaseBase {
			uint m_a;
			constructor(uint a) public {
				m_a = a;
			}
		}
		contract Base is BaseBase(7) {
			constructor() public {
				m_a *= m_a;
			}
		}
		contract Derived is Base() {
			function getA() public returns (uint r) { return m_a; }
		}
	
// ----
// getA() -> 0x31

