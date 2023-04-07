contract C {
    uint public g = 1.5 gwei;
    uint public e = 1.5 ether;
    uint public m = 1.5 minutes;
    uint public h = 1.5 hours;
    uint public d = 1.5 days;
    uint public w = 1.5 weeks;
}
// ----
// g() -> 1500000000
// e() -> 1500000000000000000
// m() -> 90
// h() -> 5400
// d() -> 129600
// w() -> 907200
