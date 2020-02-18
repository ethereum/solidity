
		contract c {
			struct Data { uint x; uint y; }
			Data[] data;
			uint[] ids;
			function setIDStatic(uint id) public { ids[2] = id; }
			function setID(uint index, uint id) public { ids[index] = id; }
			function setData(uint index, uint x, uint y) public { data[index].x = x; data[index].y = y; }
			function getID(uint index) public returns (uint) { return ids[index]; }
			function getData(uint index) public returns (uint x, uint y) { x = data[index].x; y = data[index].y; }
			function getLengths() public returns (uint l1, uint l2) { l1 = data.length; l2 = ids.length; }
			function setLengths(uint l1, uint l2) public {
				while (data.length < l1)
					data.push();
				while (ids.length < l2)
					ids.push();
			}
		}
	
// ----
// getLengths() -> 0x0, 0x0
// setLengths(uint256,uint256): 0x30, 0x31 -> 
// getLengths() -> 0x30, 0x31
// setIDStatic(uint256): 0xb -> 
// getID(uint256): 0x2 -> 0xb
// setID(uint256,uint256): 0x7, 0x8 -> 
// getID(uint256): 0x7 -> 0x8
// setData(uint256,uint256,uint256): 0x7, 0x8, 0x9 -> 
// setData(uint256,uint256,uint256): 0x8, 0xa, 0xb -> 
// getData(uint256): 0x7 -> 0x8, 0x9
// getData(uint256): 0x8 -> 0xa, 0xb

