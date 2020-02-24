
		contract Base {
			constructor(uint i) public
			{
				m_i = i;
			}
			uint public m_i;
		}
		abstract contract Base1 is Base {
			constructor(uint k) public {}
		}
		contract Derived is Base, Base1 {
			constructor(uint i) Base(i) Base1(7) public {}
		}
		contract Final is Derived(4) {
		}
	
// ----
// m_i() -> 4

