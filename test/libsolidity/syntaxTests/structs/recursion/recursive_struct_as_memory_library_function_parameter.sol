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
// TypeError 4103: (146-168): Recursive structs can only be passed as storage pointers to libraries, not as memory objects to contract functions.
