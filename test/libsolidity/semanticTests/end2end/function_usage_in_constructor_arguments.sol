
		contract BaseBase {
			uint m_a;
			constructor(uint a) public {
				m_a = a;
			}
			function g() public returns (uint r) { return 2; }
		}
		contract Base is BaseBase(BaseBase.g()) {
		}
		contract Derived is Base() {
			function getA() public returns (uint r) { return m_a; }
		}
	
// ----
// getA() -> 0x2

