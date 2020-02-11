// start

pragma experimental ABIEncoderV2;
contract C {
    struct S {
        uint a;
        uint[] b;
    }

    function f(bytes calldata data) external pure returns(S memory) {
        return abi.decode(data, (S));
    }
}

// ----
// f(bytes): 32, 224, 32, 33, 64, 3, 10, 11, 12 -> 0x20, 33, 0x40, 3, 10, 11, 12
