contract C {
    mapping (uint => uint) m;
    uint a = m(1000);
}
// ----
// TypeError 5704: (56-63): Mapping is not callable.
