==== Source: C ====
import "X";
import "V1A";
import "V2A";
import "V1B";

contract C is V1A, V2A, V1B {
    function foo()
        public
        modV1A
        modV2A // There should be no error for modV2A (it uses ABIEncoderV2)
        modV1B
    {
    }
}
==== Source: V1A ====
import "X";

contract V1A {
    modifier modV1A() {
        _;
    }
}
==== Source: V1B ====
import "X";

contract V1B {
    modifier modV1B() {
        _;
    }
}
==== Source: V2A ====
pragma experimental ABIEncoderV2;
import "X";

contract V2A {
    modifier modV2A() {
        X(0x00).get();
        _;
    }
}
==== Source: X ====
pragma experimental ABIEncoderV2;

struct Data {
    bool flag;
}

contract X {
    function get() public view returns (Data memory) {}
}
// ----
