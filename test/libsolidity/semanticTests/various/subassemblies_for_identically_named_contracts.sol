==== Source: D1.sol ====
contract D {
    function f() public pure returns (uint) {
        return 42;
    }
}
==== Source: D2.sol ====
contract D {
    function f() public pure returns (uint) {
        return 66;
    }
}
==== Source: C.sol ====
import {D as D1} from "D1.sol";
import {D as D2} from "D2.sol";

contract C {
    // The purpose of the test is to ensure that there is no naming conflict between subobjects
    // that the codegen will produce from contracts with identical names and that both get included.
    D1 d1 = new D1();
    D2 d2 = new D2();

    function test() public returns (uint, uint) {
        return (d1.f(), d2.f());
    }
}
// ----
// test() -> 42, 66
