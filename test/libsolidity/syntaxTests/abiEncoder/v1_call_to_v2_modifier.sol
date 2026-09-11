==== Source: A ====
pragma abicoder               v2;

struct Data {
    bool flag;
}

contract A {
    function get() public view returns (Data memory) {}
}

contract B {
    modifier validate() {
        A(address(0x00)).get();
        _;
    }
}
==== Source: B ====
pragma abicoder v1;
import "A";

contract C is B {
    function foo()
        public
        validate()
    {}
}
// ====
// compileViaYul: false
// ----
// Warning 9511: (B:0-19): ABI coder v1 is deprecated and scheduled for removal. Use ABI coder v2 instead.
