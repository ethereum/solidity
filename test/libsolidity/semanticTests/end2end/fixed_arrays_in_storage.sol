
		contract c {
			struct Data { uint x; uint y; }
			Data[2**10] data;
			uint[2**10 + 3] ids;
			function setIDStatic(uint id) public { ids[2] = id; }
			function setID(uint index, uint id) public { ids[index] = id; }
			function setData(uint index, uint x, uint y) public { data[index].x = x; data[index].y = y; }
			function getID(uint index) public returns (uint) { return ids[index]; }
			function getData(uint index) public returns (uint x, uint y) { x = data[index].x; y = data[index].y; }
			function getLengths() public returns (uint l1, uint l2) { l1 = data.length; l2 = ids.length; }
		}
	
// ----
// setIDStatic(uint256): 0xb -> 
// getID(uint256): 0x2 -> 0xb
// setID(uint256,uint256): 0x7, 0x8 -> 
// getID(uint256): 0x7 -> 0x8
// setData(uint256,uint256,uint256): 0x7, 0x8, 0x9 -> 
// setData(uint256,uint256,uint256): 0x8, 0xa, 0xb -> 
// getData(uint256): 0x7 -> 0x8, 0x9
// getData(uint256): 0x8 -> 0xa, 0xb
// getLengths() -> 0x400, 0x403

