==== Source: A.sol ====
error ErrorA(string msg);
==== Source: B.sol ====
import * as A from "A.sol";

error ErrorB(string msg);

contract BContract {
    error ErrorB(string msg);
}
==== Source: C.sol ====
import * as B from "B.sol";

contract CContract {
    function error1() external {
        revert B.ErrorB("B error");
    }

    function error2() external {
        revert B.BContract.ErrorB("B.BContract error");
    }

    function error3() external {
        revert B.A.ErrorA("B.A error");
    }
}

// ====
// targetContract: CContract
// ----
// error1() -> FAILURE, hex"a5f9ec67", 0x20, 7, "B error"
// error2() -> FAILURE, hex"a5f9ec67", 0x20, 17, "B.BContract error"
// error3() -> FAILURE, hex"23b0db14", 0x20, 9, "B.A error"
