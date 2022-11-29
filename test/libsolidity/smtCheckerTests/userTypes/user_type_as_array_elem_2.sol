contract C {
    type T is address;
    T[] arr;

    function p() public {
        arr.push(T.wrap(address(42)));
    }

    function inv2() external view {
        if (arr.length > 0) {
            assert(T.unwrap(arr[0]) == address(0)); // should fail
        }
    }
}
// ----
// Warning 6328: (200-238): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.p()\nC.inv2()
