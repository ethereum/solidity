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
// TypeError: (113-124): This variable is of storage pointer type and can be returned without prior assignment, which would lead to undefined behaviour.
// TypeError: (240-251): This variable is of storage pointer type and can be returned without prior assignment, which would lead to undefined behaviour.
// TypeError: (367-378): This variable is of storage pointer type and can be returned without prior assignment, which would lead to undefined behaviour.
// TypeError: (631-632): This variable is of storage pointer type and can be accessed without prior assignment, which would lead to undefined behaviour.
