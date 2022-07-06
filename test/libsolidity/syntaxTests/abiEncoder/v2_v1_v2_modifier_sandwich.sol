==== Source: A ====
pragma abicoder               v2;

struct Data {
    bool flag;
}

contract A {
    function get() public view returns (Data memory) {}
}
==== Source: B ====
pragma abicoder v1;
import "A";

contract B {
    modifier validate() {
        A(address(0x00)).get();
        _;
    }
}
==== Source: C ====
pragma abicoder               v2;

import "B";

contract C is B {
    function foo()
        public
        validate()
    {}
}
// ----
// TypeError 2428: (B:80-102): The type of return parameter 1, struct Data memory, is only supported in ABI coder v2. Use "pragma abicoder v2;" to enable the feature.
