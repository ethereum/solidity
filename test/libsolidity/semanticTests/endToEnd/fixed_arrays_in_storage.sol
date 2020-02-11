contract c {
    struct Data {
        uint x;
        uint y;
    }
    Data[2 ** 10] data;
    uint[2 ** 10 + 3] ids;

    function setIDStatic(uint id) public {
        ids[2] = id;
    }

    function setID(uint index, uint id) public {
        ids[index] = id;
    }

    function setData(uint index, uint x, uint y) public {
        data[index].x = x;
        data[index].y = y;
    }

    function getID(uint index) public returns(uint) {
        return ids[index];
    }

    function getData(uint index) public returns(uint x, uint y) {
        x = data[index].x;
        y = data[index].y;
    }

    function getLengths() public returns(uint l1, uint l2) {
        l1 = data.length;
        l2 = ids.length;
    }
}

// ----
// setIDStatic(uint256): 11 -> bytes(
// setIDStatic(uint256):"11" -> ""
// getID(uint256): 2 -> 11
// getID(uint256):"2" -> "11"
// setID(uint256,uint256): 7, 8 -> bytes(
// setID(uint256,uint256):"7, 8" -> ""
// getID(uint256): 7 -> 8
// getID(uint256):"7" -> "8"
// setData(uint256,uint256,uint256): 7, 8, 9 -> bytes(
// setData(uint256,uint256,uint256):"7, 8, 9" -> ""
// setData(uint256,uint256,uint256): 8, 10, 11 -> bytes(
// setData(uint256,uint256,uint256):"8, 10, 11" -> ""
// getData(uint256): 7 -> 8, 9
// getData(uint256):"7" -> "8, 9"
// getData(uint256): 8 -> 10, 11
// getData(uint256):"8" -> "10, 11"
// getLengths() -> 1 << 10, (1 << 10 + 3
// getLengths():"" -> "1024, 1027"
