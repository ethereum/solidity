struct S {
    uint256 v;
}

contract C
{
    S private s;
    uint256 v = s.v;
    uint256 constant vConst = s.v;
}
// ----
// TypeError 8349: (110-113): Initial value for constant variable has to be compile-time constant.
