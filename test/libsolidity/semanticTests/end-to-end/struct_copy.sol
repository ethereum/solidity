
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
// set(uint256): 7 -> true
// retrieve(uint256): 7 -> 1, 3, 4, 2
// copy(uint256,uint256): 7, 8 -> true
// retrieve(uint256): 7 -> 1, 3, 4, 2
// retrieve(uint256): 8 -> 1, 3, 4, 2
// copy(uint256,uint256): 0, 7 -> true
// retrieve(uint256): 7 -> 0, 0, 0, 0
// retrieve(uint256): 8 -> 1, 3, 4, 2
// copy(uint256,uint256): 7, 8 -> true
// retrieve(uint256): 8 -> 0, 0, 0, 0

