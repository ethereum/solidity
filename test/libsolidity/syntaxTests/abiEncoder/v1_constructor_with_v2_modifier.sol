==== Source: A ====
pragma abicoder               v2;

struct Data {
    bool flag;
}

contract A {
    function get() public view returns (Data memory) {}
}

contract B {
    constructor() validate {
        A(address(0x00)).get();
    }

    modifier validate() {
        A(address(0x00)).get();
        _;
    }
}

==== Source: B ====
pragma abicoder v1;
import "A";

contract C is B {}
==== Source: C ====
pragma abicoder v1;
import "B";

contract D is C {
    constructor() validate B() validate C() validate {}
}
// ----
