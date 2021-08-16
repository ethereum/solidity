contract C {
    fixed x = 1.7;
    fixed immutable y = -1.8;
    fixed public z = - 9.3;
    fixed constant w = -3.4;
    function f() public view returns (fixed, fixed, fixed, fixed) {
        return (x, y, this.z(), w);
    }
}
// ----
// z() -> -9.300000000000000000
// f() -> 1.700000000000000000, -1.800000000000000000, -9.300000000000000000, -3.400000000000000000
