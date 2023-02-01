contract C {
    type T is address;
    T[] arr;

    function p() public {
        arr.push(T.wrap(address(42)));
    }

    function inv(uint i) external view {
        require(i < arr.length);
        assert(T.unwrap(arr[i]) == address(42)); // should hold
    }
}
// ====
// SMTIgnoreOS: macos
// ----
// Warning 6328: (204-243): CHC: Assertion violation might happen here.
// Warning 4661: (204-243): BMC: Assertion violation happens here.
