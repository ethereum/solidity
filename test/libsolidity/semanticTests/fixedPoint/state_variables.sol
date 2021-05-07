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
// z() ->
// f() ->
