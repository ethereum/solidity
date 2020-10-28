==== Source: ERC20.sol ====
pragma experimental SMTChecker;
contract ERC20 {
    uint256 private a;
    function f() internal virtual {
        a = 2;
    }
}
==== Source: Token.sol ====
pragma experimental SMTChecker;
import "ERC20.sol";
contract Token is ERC20 {
    constructor() {
      f();
    }
}
