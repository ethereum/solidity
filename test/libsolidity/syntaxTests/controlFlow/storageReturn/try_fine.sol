contract C {
    struct S { bool f; }
    S s;
    function ext() external { }
    function f() internal returns (S storage r)
    {
        try this.ext() { r = s; }
        catch (bytes memory) { r = s; }
    }
    function g() internal returns (S storage r)
    {
        try this.ext() { r = s; }
        catch Error (string memory) { r = s; }
        catch (bytes memory) { r = s; }
    }
    function h() internal returns (S storage r)
    {
        try this.ext() { }
        catch (bytes memory) { }
        r = s;
    }
}
// ====
// EVMVersion: >=byzantium