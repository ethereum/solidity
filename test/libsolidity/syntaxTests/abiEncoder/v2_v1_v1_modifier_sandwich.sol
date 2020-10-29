==== Source: A ====
pragma experimental ABIEncoderV2;

struct Data {
    bool flag;
}

contract A {
    function get() public view returns (Data memory) {}
}
==== Source: B ====
import "A";

contract B {
    modifier validate() {
        A(0x00).get();
        _;
    }
}
==== Source: C ====
import "B";

contract C is B {
    function foo()
        public
        validate()
    {}
}
// ----
// TypeError 2428: (B:60-73): The type of return parameter 1, struct Data, is only supported in ABI coder v2. Use "pragma abicoder v2;" to enable the feature.
