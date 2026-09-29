contract C {
    struct S { uint256 v; }
    S[20] public a;

    // g(0, 2, 0xaaaa, 0xbbbb) must return 0xbbbb (i == k after the second loop)
    function g(uint256 n1, uint256 n2, uint256 v, uint256 w) external returns (uint256) {
        uint256 i;
        for (uint256 j; j < n1; j++) i++;
        uint256 k = i + 2;
        a[k].v = v;
        for (uint256 j; j < n2; j++) i++;
        a[i].v = w;
        return a[k].v;
    }

    // rmw(0, 1) must leave a[1].v == 501 (i == k after the second loop)
    function rmw(uint256 n1, uint256 n2) external {
        uint256 i;
        for (uint256 j; j < n1; j++) i++;
        uint256 k = i + 1;
        a[k].v = 100;
        for (uint256 j; j < n2; j++) i++;
        a[i].v = 500;
        a[k].v += 1;
    }
}
// ----
// g(uint256,uint256,uint256,uint256): 0, 2, 0xaaaa, 0xbbbb -> 0xbbbb
// g(uint256,uint256,uint256,uint256): 0, 3, 0xaaaa, 0xbbbb -> 0xaaaa
// rmw(uint256,uint256): 0, 1 ->
// a(uint256): 1 -> 501
