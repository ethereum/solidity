contract C {
    struct S { bool f; }
    S s;
    function f(uint256 a) internal pure returns (S storage c) {
        assembly {
            switch a
                default { c.slot := s.slot }
        }
    }
}
// ----
// Warning 9592: (142-195): "switch" statement with only a default case.
