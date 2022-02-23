contract C {
    struct S { bool f; }
    S s;
    function ext() external { }
    function f() internal
    {
        S storage r;
        try this.ext() { r = s; }
        catch (bytes memory) { r = s; }
        r;
    }
    function g() internal
    {
        S storage r;
        try this.ext() { r = s; }
        catch Error (string memory) { r = s; }
        catch (bytes memory) { r = s; }
        r;
    }
    function h() internal
    {
        S storage r;
        try this.ext() { }
        catch (bytes memory) { }
        r = s;
        r;
    }
}
// ====
// EVMVersion: >=byzantium
// ----
