==== Source: A ====
pragma experimental ABIEncoderV2;

struct Data {
    bool flag;
}

contract A {
    function get() public view returns (Data memory) {}
}

contract B {
    modifier validate() {
        A(0x00).get();
        _;
    }
}
==== Source: B ====
import "A";

contract C is B {
    function foo()
        public
        validate()
    {}
}
// ----
