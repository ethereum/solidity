==== Source: A.sol ====
import "@/D.sol";
import "B.sol";

contract A is B {
    function a() public pure {
        e();
    }
}

==== Source: B.sol ====
import "C.sol";

abstract contract B is C {}

==== Source: C.sol ====
abstract contract C {
    function c() public pure returns (uint) {
        return 0;
    }
}

==== Source: @/D.sol ====
import "@/E.sol";

==== Source: @/E.sol ====
function e() pure returns (bytes memory returndata) {
    return "";
}
