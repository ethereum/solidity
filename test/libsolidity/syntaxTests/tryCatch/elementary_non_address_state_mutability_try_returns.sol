contract C {
    function a() public pure returns (bool) {
        try this.a() returns (bool payable res) {} catch {}
    }
    function b() public pure returns (string) {
        try this.b() returns (string payable res) {} catch {}
    }
    function c() public pure returns (int) {
        try this.c() returns (int payable res) {} catch {}
    }
    function d() public pure returns (int256) {
        try this.d() returns (int256 payable res) {} catch {}
    }
    function e() public pure returns (uint) {
        try this.e() returns (uint payable res) {} catch {}
    }
    function f() public pure returns (uint256) {
        try this.f() returns (uint256 payable res) {} catch {}
    }
    function g() public pure returns (bytes1) {
        try this.g() returns (bytes1 payable res) {} catch {}
    }
    function h() public pure returns (bytes) {
        try this.h() returns (bytes payable res) {} catch {}
    }
    function i() public pure returns (bytes32) {
        try this.i() returns (bytes32 payable res) {} catch {}
    }
    function j() public pure returns (fixed) {
        try this.j() returns (fixed payable res) {} catch {}
    }
    function k() public pure returns (fixed80x80) {
        try this.k() returns (fixed80x80 payable res) {} catch {}
    }
    function l() public pure returns (ufixed) {
        try this.l() returns (ufixed payable res) {} catch {}
    }
    function m() public pure returns (ufixed80x80) {
        try this.m() returns (ufixed80x80 payable res) {} catch {}
    }
}
contract C1 {
    function a() public pure returns (bool) {
        try this.a() returns (bool view res) {} catch {}
    }
    function b() public pure returns (string) {
        try this.b() returns (string view res) {} catch {}
    }
    function c() public pure returns (int) {
        try this.c() returns (int view res) {} catch {}
    }
    function d() public pure returns (int256) {
        try this.d() returns (int256 view res) {} catch {}
    }
    function e() public pure returns (uint) {
        try this.e() returns (uint view res) {} catch {}
    }
    function f() public pure returns (uint256) {
        try this.f() returns (uint256 view res) {} catch {}
    }
    function g() public pure returns (bytes1) {
        try this.g() returns (bytes1 view res) {} catch {}
    }
    function h() public pure returns (bytes) {
        try this.h() returns (bytes view res) {} catch {}
    }
    function i() public pure returns (bytes32) {
        try this.i() returns (bytes32 view res) {} catch {}
    }
    function j() public pure returns (fixed) {
        try this.j() returns (fixed view res) {} catch {}
    }
    function k() public pure returns (fixed80x80) {
        try this.k() returns (fixed80x80 view res) {} catch {}
    }
    function l() public pure returns (ufixed) {
        try this.l() returns (ufixed view res) {} catch {}
    }
    function m() public pure returns (ufixed80x80) {
        try this.m() returns (ufixed80x80 view res) {} catch {}
    }
}
contract C2 {
    function a() public pure returns (bool) {
        try this.a() returns (bool pure res) {} catch {}
    }
    function b() public pure returns (string) {
        try this.b() returns (string pure res) {} catch {}
    }
    function c() public pure returns (int) {
        try this.c() returns (int pure res) {} catch {}
    }
    function d() public pure returns (int256) {
        try this.d() returns (int256 pure res) {} catch {}
    }
    function e() public pure returns (uint) {
        try this.e() returns (uint pure res) {} catch {}
    }
    function f() public pure returns (uint256) {
        try this.f() returns (uint256 pure res) {} catch {}
    }
    function g() public pure returns (bytes1) {
        try this.g() returns (bytes1 pure res) {} catch {}
    }
    function h() public pure returns (bytes) {
        try this.h() returns (bytes pure res) {} catch {}
    }
    function i() public pure returns (bytes32) {
        try this.i() returns (bytes32 pure res) {} catch {}
    }
    function j() public pure returns (fixed) {
        try this.j() returns (fixed pure res) {} catch {}
    }
    function k() public pure returns (fixed80x80) {
        try this.k() returns (fixed80x80 pure res) {} catch {}
    }
    function l() public pure returns (ufixed) {
        try this.l() returns (ufixed pure res) {} catch {}
    }
    function m() public pure returns (ufixed80x80) {
        try this.m() returns (ufixed80x80 pure res) {} catch {}
    }
}
// ----
// ParserError 9106: (94-101): State mutability can only be specified for address types.
// ParserError 9106: (210-217): State mutability can only be specified for address types.
// ParserError 9106: (320-327): State mutability can only be specified for address types.
// ParserError 9106: (436-443): State mutability can only be specified for address types.
// ParserError 9106: (548-555): State mutability can only be specified for address types.
// ParserError 9106: (666-673): State mutability can only be specified for address types.
// ParserError 9106: (782-789): State mutability can only be specified for address types.
// ParserError 9106: (896-903): State mutability can only be specified for address types.
// ParserError 9106: (1014-1021): State mutability can only be specified for address types.
// ParserError 9106: (1128-1135): State mutability can only be specified for address types.
// ParserError 9106: (1252-1259): State mutability can only be specified for address types.
// ParserError 9106: (1368-1375): State mutability can only be specified for address types.
// ParserError 9106: (1494-1501): State mutability can only be specified for address types.
// ParserError 9106: (1622-1626): State mutability can only be specified for address types.
// ParserError 9106: (1735-1739): State mutability can only be specified for address types.
// ParserError 9106: (1842-1846): State mutability can only be specified for address types.
// ParserError 9106: (1955-1959): State mutability can only be specified for address types.
// ParserError 9106: (2064-2068): State mutability can only be specified for address types.
// ParserError 9106: (2179-2183): State mutability can only be specified for address types.
// ParserError 9106: (2292-2296): State mutability can only be specified for address types.
// ParserError 9106: (2403-2407): State mutability can only be specified for address types.
// ParserError 9106: (2518-2522): State mutability can only be specified for address types.
// ParserError 9106: (2629-2633): State mutability can only be specified for address types.
// ParserError 9106: (2750-2754): State mutability can only be specified for address types.
// ParserError 9106: (2863-2867): State mutability can only be specified for address types.
// ParserError 9106: (2986-2990): State mutability can only be specified for address types.
// ParserError 9106: (3111-3115): State mutability can only be specified for address types.
// ParserError 9106: (3224-3228): State mutability can only be specified for address types.
// ParserError 9106: (3331-3335): State mutability can only be specified for address types.
// ParserError 9106: (3444-3448): State mutability can only be specified for address types.
// ParserError 9106: (3553-3557): State mutability can only be specified for address types.
// ParserError 9106: (3668-3672): State mutability can only be specified for address types.
// ParserError 9106: (3781-3785): State mutability can only be specified for address types.
// ParserError 9106: (3892-3896): State mutability can only be specified for address types.
// ParserError 9106: (4007-4011): State mutability can only be specified for address types.
// ParserError 9106: (4118-4122): State mutability can only be specified for address types.
// ParserError 9106: (4239-4243): State mutability can only be specified for address types.
// ParserError 9106: (4352-4356): State mutability can only be specified for address types.
// ParserError 9106: (4475-4479): State mutability can only be specified for address types.
