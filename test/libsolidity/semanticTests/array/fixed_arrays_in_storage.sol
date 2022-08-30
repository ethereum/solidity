contract c {
    struct Data {
        uint256 x;
        uint256 y;
    }
    Data[2**10] data;
    uint256[2**10 + 3] ids;

    function setIDStatic(uint256 id) public {
        ids[2] = id;
    }

    function setID(uint256 index, uint256 id) public {
        ids[index] = id;
    }

    function setData(uint256 index, uint256 x, uint256 y) public {
        data[index].x = x;
        data[index].y = y;
    }

    function getID(uint256 index) public returns (uint256) {
        return ids[index];
    }

    function getData(uint256 index) public returns (uint256 x, uint256 y) {
        x = data[index].x;
        y = data[index].y;
    }

    function getLengths() public returns (uint256 l1, uint256 l2) {
        l1 = data.length;
        l2 = ids.length;
    }
}
// ====
// compileToEwasm: also
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
