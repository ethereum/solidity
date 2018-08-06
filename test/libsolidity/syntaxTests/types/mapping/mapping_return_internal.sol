// This should be allowed in a future release.
contract C {
    mapping(uint=>uint) m;
    function f() internal view returns (mapping(uint=>uint) storage) {
        return m;
    }
    function g() private view returns (mapping(uint=>uint) storage) {
        return m;
    }
    function h() internal view returns (mapping(uint=>uint) storage r) {
        r = m;
    }
    function i() private view returns (mapping(uint=>uint) storage r) {
        (r,r) = (m,m);
    }
}
// ----
// TypeError: (127-146): Type is required to live outside storage.
// TypeError: (221-240): Type is required to live outside storage.
// TypeError: (316-345): Type is required to live outside storage.
// TypeError: (409-438): Type is required to live outside storage.
