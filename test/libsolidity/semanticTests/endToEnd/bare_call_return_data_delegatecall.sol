contract A {
    constructor() public {}

    function return_bool() public pure returns(bool) {
        return true;
    }

    function return_int32() public pure returns(int32) {
        return -32;
    }

    function return_uint32() public pure returns(uint32) {
        return 0x3232;
    }

    function return_int256() public pure returns(int256) {
        return -256;
    }

    function return_uint256() public pure returns(uint256) {
        return 0x256256;
    }

    function return_bytes4() public pure returns(bytes4) {
        return 0xabcd0012;
    }

    function return_multi() public pure returns(bool, uint32, bytes4) {
        return (false, 0x3232, 0xabcd0012);
    }

    function return_bytes() public pure returns(bytes memory b) {
        b = new bytes(2);
        b[0] = 0x42;
        b[1] = 0x21;
    }
}
contract C {
    A addr;
    constructor() public {
        addr = new A();
    }

    function f(string memory signature) public returns(bool, bytes memory) {
        return address(addr).delegatecall(abi.encodeWithSignature(signature));
    }

    function check_bool() external returns(bool) {
        (bool success, bytes memory data) = f("return_bool()");
        assert(success);
        bool a = abi.decode(data, (bool));
        assert(a);
        return true;
    }

    function check_int32() external returns(bool) {
        (bool success, bytes memory data) = f("return_int32()");
        assert(success);
        int32 a = abi.decode(data, (int32));
        assert(a == -32);
        return true;
    }

    function check_uint32() external returns(bool) {
        (bool success, bytes memory data) = f("return_uint32()");
        assert(success);
        uint32 a = abi.decode(data, (uint32));
        assert(a == 0x3232);
        return true;
    }

    function check_int256() external returns(bool) {
        (bool success, bytes memory data) = f("return_int256()");
        assert(success);
        int256 a = abi.decode(data, (int256));
        assert(a == -256);
        return true;
    }

    function check_uint256() external returns(bool) {
        (bool success, bytes memory data) = f("return_uint256()");
        assert(success);
        uint256 a = abi.decode(data, (uint256));
        assert(a == 0x256256);
        return true;
    }

    function check_bytes4() external returns(bool) {
        (bool success, bytes memory data) = f("return_bytes4()");
        assert(success);
        bytes4 a = abi.decode(data, (bytes4));
        assert(a == 0xabcd0012);
        return true;
    }

    function check_multi() external returns(bool) {
        (bool success, bytes memory data) = f("return_multi()");
        assert(success);
        (bool a, uint32 b, bytes4 c) = abi.decode(data, (bool, uint32, bytes4));
        assert(a == false && b == 0x3232 && c == 0xabcd0012);
        return true;
    }

    function check_bytes() external returns(bool) {
        (bool success, bytes memory data) = f("return_bytes()");
        assert(success);
        (bytes memory d) = abi.decode(data, (bytes));
        assert(d.length == 2 && d[0] == 0x42 && d[1] == 0x21);
        return true;
    }
}

// ----
// f(string): "return_bool()" -> FAILURE
// f(string): "return_int32())" -> FAILURE
// f(string): "return_uint32()" -> FAILURE
// f(string): "return_int256()" -> FAILURE
// f(string): "return_uint256()" -> FAILURE
// f(string): "return_bytes4()" -> FAILURE
// f(string): "return_multi()" -> FAILURE
// f(string): "return_bytes()" -> FAILURE
// check_bool() -> true
// check_int32() -> true
// check_uint32() -> true
// check_int256() -> true
// check_uint256() -> true
// check_bytes4() -> true
// check_multi() -> true
// check_bytes() -> true
