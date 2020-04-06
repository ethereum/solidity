contract C {
    struct S { bool f; }
    S s;
    function ext() external {}
    function f() internal
    {
        S storage r;
        try this.ext() { }
        catch (bytes memory) { r = s; }
        r;
    }
    function g() internal
    {
        S storage r;
        try this.ext() { r = s; }
        catch (bytes memory) { }
        r;
    }
    function h() internal
    {
        S storage r;
        try this.ext() {}
        catch Error (string memory) { r = s; }
        catch (bytes memory) { r = s; }
        r;
    }
    function i() internal
    {
        S storage r;
        try this.ext() { r = s; }
        catch (bytes memory) { r; }
        r = s;
        r;
    }
}
// ====
// EVMVersion: >=byzantium
// ----
// TypeError: (206-207): This variable is of storage pointer type and can be accessed without prior assignment, which would lead to undefined behaviour.
// TypeError: (343-344): This variable is of storage pointer type and can be accessed without prior assignment, which would lead to undefined behaviour.
// TypeError: (526-527): This variable is of storage pointer type and can be accessed without prior assignment, which would lead to undefined behaviour.
// TypeError: (653-654): This variable is of storage pointer type and can be accessed without prior assignment, which would lead to undefined behaviour.
