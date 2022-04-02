contract C {
    struct S { bool f; }
    S s;
    function ext() external {}
    function f() internal returns (S storage r)
    {
        try this.ext() { }
        catch (bytes memory) { r = s; }
    }
    function g() internal returns (S storage r)
    {
        try this.ext() { r = s; }
        catch (bytes memory) { }
    }
    function h() internal returns (S storage r)
    {
        try this.ext() {}
        catch Error (string memory) { r = s; }
        catch (bytes memory) { r = s; }
    }
    function i() internal returns (S storage r)
    {
        try this.ext() { r = s; }
        catch (bytes memory) { return r; }
        r = s;
    }
}
// ====
// EVMVersion: >=byzantium
// ----
// TypeError 3464: (113-124='S storage r'): This variable is of storage pointer type and can be returned without prior assignment, which would lead to undefined behaviour.
// TypeError 3464: (240-251='S storage r'): This variable is of storage pointer type and can be returned without prior assignment, which would lead to undefined behaviour.
// TypeError 3464: (367-378='S storage r'): This variable is of storage pointer type and can be returned without prior assignment, which would lead to undefined behaviour.
// TypeError 3464: (631-632='r'): This variable is of storage pointer type and can be accessed without prior assignment, which would lead to undefined behaviour.
