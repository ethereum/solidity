==== Source: A ====
pragma experimental ABIEncoderV2;

struct Data {
    bool flag;
}

contract A {
    function get() public view returns (Data memory) {}
}

contract B {
    constructor() validate {
        A(0x00).get();
    }

    modifier validate() {
        A(0x00).get();
        _;
    }
}

==== Source: B ====
import "A";

contract C is B {}
==== Source: C ====
import "B";

contract D is C {
    constructor() validate B() validate C() validate {}
}
// ----
