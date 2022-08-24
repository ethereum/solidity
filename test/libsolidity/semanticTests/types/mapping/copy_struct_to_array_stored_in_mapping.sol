pragma abicoder v2;

contract C {
    struct S {
        uint8 x;
    }

    constructor() {
        s = S({x: 7});
        m2[0].push();
    }

    S s;

    mapping (uint8 => S[2]) m1;
    mapping (uint8 => S[]) m2;

    function from_storage_to_static_array() public returns (S[2] memory) {
        m1[0][1] = s;
        return m1[0];
    }

    function from_storage_to_dynamic_array() public returns (S[] memory) {
        m2[0][0] = s;
        return m2[0];
    }

    function from_memory_to_static_array() public returns (S[2] memory) {
        S memory sLocal = s;
        m1[0][1] = sLocal;
        return m1[0];
    }

    function from_memory_to_dynamic_array() public returns (S[] memory) {
        S memory sLocal = s;
        m2[0][0] = sLocal;
        return m2[0];
    }

    function from_calldata_to_static_array(S calldata sCalldata) public returns (S[2] memory) {
        m1[0][1] = sCalldata;
        return m1[0];
    }

    function from_calldata_to_dynamic_array(S calldata sCalldata) public returns (S[] memory) {
        m2[0][0] = sCalldata;
        return m2[0];
    }
}

// ----
// from_storage_to_static_array() -> 0, 7
// from_storage_to_dynamic_array() -> 0x20, 1, 7
// from_memory_to_static_array() -> 0, 7
// from_memory_to_dynamic_array() -> 0x20, 1, 7
// from_calldata_to_static_array((uint8)): 8 -> 0, 8
// from_calldata_to_dynamic_array((uint8)): 8 -> 0x20, 1, 8
