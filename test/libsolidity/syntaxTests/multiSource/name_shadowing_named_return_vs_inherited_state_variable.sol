==== Source: A ====
contract A{
    uint256 value = 10;
}
==== Source: B ====
import "A";
contract B is A{}
==== Source: C ====
import "B";
contract C is B {
    function f() public pure returns (uint256 value) {
        return value;
    }
}
// ----
// Warning 2519: (C:68-81): This declaration shadows an existing declaration.