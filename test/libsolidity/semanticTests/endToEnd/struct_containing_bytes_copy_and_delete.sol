contract c {
    struct Struct {
        uint a;
        bytes data;
        uint b;
    }
    Struct data1;
    Struct data2;

    function set(uint _a, bytes calldata _data, uint _b) external returns(bool) {
        data1.a = _a;
        data1.b = _b;
        data1.data = _data;
        return true;
    }

    function copy() public returns(bool) {
        data1 = data2;
        return true;
    }

    function del() public returns(bool) {
        delete data1;
        return true;
    }
}

// ----
// set(uint256,bytes,uint256): 12, 0x60, 13, data.length()), data -> true
// set(uint256,bytes,uint256):"12, 96, 13, 33, 123456789012345678901234567890123" -> "1"
// copy() -> true
// copy():"" -> "1"
// set(uint256,bytes,uint256): 12, 0x60, 13, data.length()), data -> true
// set(uint256,bytes,uint256):"12, 96, 13, 33, 123456789012345678901234567890123" -> "1"
// del() -> true
// del():"" -> "1"
