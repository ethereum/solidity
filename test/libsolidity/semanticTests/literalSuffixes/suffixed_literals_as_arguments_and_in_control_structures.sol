function str(uint) pure suffix returns (bytes memory) {
    return "abc";
}

function len(bytes memory b) pure suffix returns (uint) {
    return b.length;
}

function hash(bytes memory input) pure suffix returns (bytes32) {
    return keccak256(input);
}

function iszero(uint x) pure suffix returns (bool) {
    return x == 0;
}

contract D {
    event Event(bytes, uint);
    error Err(bytes, uint);

    uint public counter = "" len;

    constructor(bytes memory) {}

    modifier m(bytes memory s) {
        counter += s.length;
        _;
    }

    function f(bytes memory b) external payable returns (uint) {
        return b.length;
    }
}

library L {
    function add(uint a, uint b) internal pure returns (uint) {
        return a + b;
    }
}

contract C is D(10 str) {
    constructor() m(42 str) {
        counter += 10;
    }

    using L for *;

    function testModifier() m(88 str) m(99 str) external {}

    function testCallOptions() external returns (uint) {
        return this.f{gas: gasleft() * 10 / "1234567890" len}(1234 str);
    }

    function testNamedArgs() external returns (uint) {
        return this.f({b: 12345 str});
    }

    function testEvent() external {
        emit Event(12 str, "xyzw" len);
    }

    function testError() external pure {
        revert Err(12 str, "xyzw" len);
    }

    function testMemoryAllocation() external pure returns (uint) {
        return new uint[]("12345" len).length;
    }

    function testNewContract() external returns (uint) {
        return new D{salt: "123" hash}(100 str).f(1234567 str);
    }

    function testAttachedCall() external pure returns (uint) {
        return ("abc" len).add(3);
    }

    function testCondition() external pure returns (uint) {
        if (0 iszero)
            return 1;
        else
            return 0;
    }

    function testAssert() external pure {
        assert(0 iszero);
    }

    function testRequire() external pure {
        require(0 iszero);
    }

    function testTuple() external pure returns (bool, bool) {
        return (0 iszero, 1 iszero);
    }
}
// ====
// EVMVersion: >=constantinople
// ----
// counter() -> 13
// testModifier() ->
// counter() -> 19
// testCallOptions() -> 3
// testNamedArgs() -> 3
// testEvent() ->
// ~ emit Event(bytes,uint256): 0x40, 0x04, 0x03, "abc"
// testError() -> FAILURE, hex"83e31485", 0x40, 0x04, 0x03, "abc"
// testMemoryAllocation() -> 5
// testNewContract() -> 3
// gas irOptimized: 111878
// gas legacy: 185340
// gas legacyOptimized: 118889
// testAttachedCall() -> 6
// testCondition() -> 1
// testAssert() ->
// testRequire() ->
// testTuple() -> 1, 0
