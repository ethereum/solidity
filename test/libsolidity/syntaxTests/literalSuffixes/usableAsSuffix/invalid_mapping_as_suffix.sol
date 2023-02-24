contract C {
    mapping (uint => uint) m;
    uint a = 1000 m;
}
// ----
// TypeError 5704: (56-62): This expression cannot be used as a literal suffix.
