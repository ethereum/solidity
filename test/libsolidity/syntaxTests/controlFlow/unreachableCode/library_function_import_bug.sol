==== Source: a ====
import "b";
contract Test is Bar {}
==== Source: b ====
library Foo {
    function nop() internal {}
}

contract Bar {
    function example() public returns (uint256) {
        foo();
        return 0;
    }

    function foo() public {
        Foo.nop();
    }
}
