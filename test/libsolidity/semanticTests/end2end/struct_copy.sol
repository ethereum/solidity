
		contract c {
			struct Nested { uint x; uint y; }
			struct Struct { uint a; mapping(uint => Struct) b; Nested nested; uint c; }
			mapping(uint => Struct) data;
			function set(uint k) public returns (bool) {
				data[k].a = 1;
				data[k].nested.x = 3;
				data[k].nested.y = 4;
				data[k].c = 2;
				return true;
			}
			function copy(uint from, uint to) public returns (bool) {
				data[to] = data[from];
				return true;
			}
			function retrieve(uint k) public returns (uint a, uint x, uint y, uint c)
			{
				a = data[k].a;
				x = data[k].nested.x;
				y = data[k].nested.y;
				c = data[k].c;
			}
		}
	
// ----
// set(uint256): 0x7 -> 0x1
// retrieve(uint256): 0x7 -> 0x1, 0x3, 0x4, 0x2
// copy(uint256,uint256): 0x7, 0x8 -> 0x1
// retrieve(uint256): 0x7 -> 0x1, 0x3, 0x4, 0x2
// retrieve(uint256): 0x8 -> 0x1, 0x3, 0x4, 0x2
// copy(uint256,uint256): 0x0, 0x7 -> 0x1
// retrieve(uint256): 0x7 -> 0x0, 0x0, 0x0, 0x0
// retrieve(uint256): 0x8 -> 0x1, 0x3, 0x4, 0x2
// copy(uint256,uint256): 0x7, 0x8 -> 0x1
// retrieve(uint256): 0x8 -> 0x0, 0x0, 0x0, 0x0

