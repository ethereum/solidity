
		contract BaseBase {
			uint m_a;
			constructor(uint a) public {
				m_a = a;
			}
			function overridden() public virtual returns (uint r) { return 1; }
			function g() public returns (uint r) { return overridden(); }
		}
		contract Base is BaseBase(BaseBase.g()) {
		}
		contract Derived is Base() {
			function getA() public returns (uint r) { return m_a; }
			function overridden() public override returns (uint r) { return 2; }
		}
	
// ----
// getA() -> 2

