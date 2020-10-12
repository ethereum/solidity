==== Source: A ====
pragma experimental ABIEncoderV2;

struct Data {
    bool flag;
}

contract A {
    function get() public view returns (Data memory) {}
}

contract B {
    constructor() {
        A(0x00).get();
    }

    function foo() public view {
        A(0x00).get();
    }
}
==== Source: B ====
import "A";

contract C is B {}
// ----
