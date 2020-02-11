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
        return address(addr).call(abi.encodeWithSignature(signature));
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
// f(string): encodeDyn(string("return_bool())) -> true, 0x40, 0x20, true
// f(string):"[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,d,72,65,74,75,72,6e,5f,62,6f,6f,6c,28,29,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]" -> "1, 64, 32, 1"
// f(string): encodeDyn(string("return_int32())) -> true, 0x40, 0x20, -32
// f(string):"[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,e,72,65,74,75,72,6e,5f,69,6e,74,33,32,28,29,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]" -> "1, 64, 32, 115792089237316195423570985008687907853269984665640564039457584007913129639904"
// f(string): encodeDyn(string("return_uint32())) -> true, 0x40, 0x20, 0x3232
// f(string):"[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,f,72,65,74,75,72,6e,5f,75,69,6e,74,33,32,28,29,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]" -> "1, 64, 32, 12850"
// f(string): encodeDyn(string("return_int256())) -> true, 0x40, 0x20, -256
// f(string):"[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,f,72,65,74,75,72,6e,5f,69,6e,74,32,35,36,28,29,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]" -> "1, 64, 32, 115792089237316195423570985008687907853269984665640564039457584007913129639680"
// f(string): encodeDyn(string("return_uint256())) -> true, 0x40, 0x20, 0x256256
// f(string):"[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,10,72,65,74,75,72,6e,5f,75,69,6e,74,32,35,36,28,29,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]" -> "1, 64, 32, 2450006"
// f(string): encodeDyn(string("return_bytes4())) -> true, 0x40, 0x20, 0xabcd0012 << (28*8
// f(string):"[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,f,72,65,74,75,72,6e,5f,62,79,74,65,73,34,28,29,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]" -> "1, 64, 32, 77707701241297161208114523684180570810961921393955197002876666998915694854144"
// f(string): encodeDyn(string("return_multi())) -> true, 0x40, 0x60, false, 0x3232, 0xabcd0012 << (28*8
// f(string):"[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,e,72,65,74,75,72,6e,5f,6d,75,6c,74,69,28,29,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]" -> "1, 64, 96, 0, 12850, 77707701241297161208114523684180570810961921393955197002876666998915694854144"
// f(string): encodeDyn(string("return_bytes())) -> true, 0x40, 0x60, 0x20, 0x02, encode(bytes{0x42,0x21}, false
// f(string):"[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,e,72,65,74,75,72,6e,5f,62,79,74,65,73,28,29,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]" -> "1, 64, 96, 32, 2, [42,21,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]"
// check_bool() -> true
// check_bool():"" -> "1"
// check_int32() -> true
// check_int32():"" -> "1"
// check_uint32() -> true
// check_uint32():"" -> "1"
// check_int256() -> true
// check_int256():"" -> "1"
// check_uint256() -> true
// check_uint256():"" -> "1"
// check_bytes4() -> true
// check_bytes4():"" -> "1"
// check_multi() -> true
// check_multi():"" -> "1"
// check_bytes() -> true
// check_bytes():"" -> "1"

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
// f(string): encodeDyn(string("return_bool())) -> true, 0x40, 0x20, true
// f(string):"[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,d,72,65,74,75,72,6e,5f,62,6f,6f,6c,28,29,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]" -> "1, 64, 32, 1"
// f(string): encodeDyn(string("return_int32())) -> true, 0x40, 0x20, -32
// f(string):"[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,e,72,65,74,75,72,6e,5f,69,6e,74,33,32,28,29,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]" -> "1, 64, 32, 115792089237316195423570985008687907853269984665640564039457584007913129639904"
// f(string): encodeDyn(string("return_uint32())) -> true, 0x40, 0x20, 0x3232
// f(string):"[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,f,72,65,74,75,72,6e,5f,75,69,6e,74,33,32,28,29,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]" -> "1, 64, 32, 12850"
// f(string): encodeDyn(string("return_int256())) -> true, 0x40, 0x20, -256
// f(string):"[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,f,72,65,74,75,72,6e,5f,69,6e,74,32,35,36,28,29,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]" -> "1, 64, 32, 115792089237316195423570985008687907853269984665640564039457584007913129639680"
// f(string): encodeDyn(string("return_uint256())) -> true, 0x40, 0x20, 0x256256
// f(string):"[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,10,72,65,74,75,72,6e,5f,75,69,6e,74,32,35,36,28,29,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]" -> "1, 64, 32, 2450006"
// f(string): encodeDyn(string("return_bytes4())) -> true, 0x40, 0x20, 0xabcd0012 << (28*8
// f(string):"[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,f,72,65,74,75,72,6e,5f,62,79,74,65,73,34,28,29,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]" -> "1, 64, 32, 77707701241297161208114523684180570810961921393955197002876666998915694854144"
// f(string): encodeDyn(string("return_multi())) -> true, 0x40, 0x60, false, 0x3232, 0xabcd0012 << (28*8
// f(string):"[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,e,72,65,74,75,72,6e,5f,6d,75,6c,74,69,28,29,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]" -> "1, 64, 96, 0, 12850, 77707701241297161208114523684180570810961921393955197002876666998915694854144"
// f(string): encodeDyn(string("return_bytes())) -> true, 0x40, 0x60, 0x20, 0x02, encode(bytes{0x42,0x21}, false
// f(string):"[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,e,72,65,74,75,72,6e,5f,62,79,74,65,73,28,29,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]" -> "1, 64, 96, 32, 2, [42,21,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]"
// check_bool() -> true
// check_bool():"" -> "1"
// check_int32() -> true
// check_int32():"" -> "1"
// check_uint32() -> true
// check_uint32():"" -> "1"
// check_int256() -> true
// check_int256():"" -> "1"
// check_uint256() -> true
// check_uint256():"" -> "1"
// check_bytes4() -> true
// check_bytes4():"" -> "1"
// check_multi() -> true
// check_multi():"" -> "1"
// check_bytes() -> true
// check_bytes():"" -> "1"

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
        return address(addr).staticcall(abi.encodeWithSignature(signature));
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
// f(string): encodeDyn(string("return_bool())) -> true, 0x40, 0x20, true
// f(string):"[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,d,72,65,74,75,72,6e,5f,62,6f,6f,6c,28,29,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]" -> "1, 64, 32, 1"
// f(string): encodeDyn(string("return_int32())) -> true, 0x40, 0x20, -32
// f(string):"[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,e,72,65,74,75,72,6e,5f,69,6e,74,33,32,28,29,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]" -> "1, 64, 32, 115792089237316195423570985008687907853269984665640564039457584007913129639904"
// f(string): encodeDyn(string("return_uint32())) -> true, 0x40, 0x20, 0x3232
// f(string):"[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,f,72,65,74,75,72,6e,5f,75,69,6e,74,33,32,28,29,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]" -> "1, 64, 32, 12850"
// f(string): encodeDyn(string("return_int256())) -> true, 0x40, 0x20, -256
// f(string):"[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,f,72,65,74,75,72,6e,5f,69,6e,74,32,35,36,28,29,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]" -> "1, 64, 32, 115792089237316195423570985008687907853269984665640564039457584007913129639680"
// f(string): encodeDyn(string("return_uint256())) -> true, 0x40, 0x20, 0x256256
// f(string):"[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,10,72,65,74,75,72,6e,5f,75,69,6e,74,32,35,36,28,29,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]" -> "1, 64, 32, 2450006"
// f(string): encodeDyn(string("return_bytes4())) -> true, 0x40, 0x20, 0xabcd0012 << (28*8
// f(string):"[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,f,72,65,74,75,72,6e,5f,62,79,74,65,73,34,28,29,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]" -> "1, 64, 32, 77707701241297161208114523684180570810961921393955197002876666998915694854144"
// f(string): encodeDyn(string("return_multi())) -> true, 0x40, 0x60, false, 0x3232, 0xabcd0012 << (28*8
// f(string):"[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,e,72,65,74,75,72,6e,5f,6d,75,6c,74,69,28,29,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]" -> "1, 64, 96, 0, 12850, 77707701241297161208114523684180570810961921393955197002876666998915694854144"
// f(string): encodeDyn(string("return_bytes())) -> true, 0x40, 0x60, 0x20, 0x02, encode(bytes{0x42,0x21}, false
// f(string):"[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,e,72,65,74,75,72,6e,5f,62,79,74,65,73,28,29,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]" -> "1, 64, 96, 32, 2, [42,21,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]"
// check_bool() -> true
// check_bool():"" -> "1"
// check_int32() -> true
// check_int32():"" -> "1"
// check_uint32() -> true
// check_uint32():"" -> "1"
// check_int256() -> true
// check_int256():"" -> "1"
// check_uint256() -> true
// check_uint256():"" -> "1"
// check_bytes4() -> true
// check_bytes4():"" -> "1"
// check_multi() -> true
// check_multi():"" -> "1"
// check_bytes() -> true
// check_bytes():"" -> "1"
