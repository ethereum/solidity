==== Source: A ====
contract C {
    function f() public returns (uint) {
        try this.f() returns (uint indexed a) {
        } catch { }
    }
}
==== Source: E ====
contract C {
    function f() public returns (uint) {
        try this.f() returns (uint) {
        } catch Error(string memory indexed x) {
        }
    }
}
==== Source: I ====
contract C {
    function f() public returns (uint) {
        try this.f() returns (uint) {
        } catch (bytes memory indexed x) {
        }
    }
}
// ----
// ParserError 2314: (A:89-96): Expected ',' but got 'indexed'
// ParserError 2314: (E:128-135): Expected ',' but got 'indexed'
// ParserError 2314: (I:122-129): Expected ',' but got 'indexed'
