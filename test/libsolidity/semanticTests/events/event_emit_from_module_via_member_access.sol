==== Source: A.sol ====
event Transfer(address indexed _from, address indexed _to, uint256 _value);
==== Source: B.sol ====
import * as A from "A.sol";

event Transfer(address indexed _from, address indexed _to, uint256 _value);

contract BContract {
    event Transfer(address indexed _from, address indexed _to, uint256 _value);
}
==== Source: C.sol ====
import * as B from "B.sol";

contract CContract {
    function returnAddress() external {
        emit B.Transfer(address(0x0b), address(0x0c), 0x0d);
        emit B.BContract.Transfer(address(0x0e), address(0x0f), 0x10);
        emit B.A.Transfer(address(0x11), address(0x12), 0x13);
    }
}

// ====
// targetContract: CContract
// ----
// returnAddress() ->
// ~ emit Transfer(address,address,uint256): #0x0b, #0x0c, 0x0d
// ~ emit Transfer(address,address,uint256): #0x0e, #0x0f, 0x10
// ~ emit Transfer(address,address,uint256): #0x11, #0x12, 0x13
