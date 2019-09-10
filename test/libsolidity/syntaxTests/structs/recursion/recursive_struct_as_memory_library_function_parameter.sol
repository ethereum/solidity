pragma experimental ABIEncoderV2;

library Test {
    struct MyStructName {
        address addr;
        MyStructName[] x;
    }

    function f(MyStructName memory _x) public {
    }
}
// ----
// Warning: (0-33): Experimental features are turned on. Do not use experimental features on live deployments.
// TypeError: (146-168): Recursive structs can only be passed as storage pointers to libraries, not as memory objects to contract functions.
