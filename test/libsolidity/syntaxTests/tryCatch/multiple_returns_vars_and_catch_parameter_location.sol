==== Source: A1 ====
contract C {
    function f() public returns (string memory) {
        try this.f() returns (string memory memory a) { } catch { }
    }
}
==== Source: A2 ====
contract C {
    function f() public returns (string memory) {
        try this.f() returns (string memory storage a) { } catch { }
    }
}
==== Source: A3 ====
contract C {
    function f() public returns (string memory) {
        try this.f() returns (string memory calldata a) { } catch { }
    }
}

==== Source: B1 ====
contract C {
    function f() public returns (string memory) {
        try this.f() returns (string calldata memory a) { } catch { }
    }
}
==== Source: B2 ====
contract C {
    function f() public returns (string memory) {
        try this.f() returns (string calldata storage a) { } catch { }
    }
}
==== Source: B3 ====
contract C {
    function f() public returns (string memory) {
        try this.f() returns (string calldata calldata a) { } catch { }
    }
}

==== Source: C1 ====
contract C {
    function f() public returns (string memory) {
        try this.f() returns (string storage memory a) { } catch { }
    }
}
==== Source: C2 ====
contract C {
    function f() public returns (string memory) {
        try this.f() returns (string storage storage a) { } catch { }
    }
}
==== Source: C3 ====
contract C {
    function f() public returns (string memory) {
        try this.f() returns (string storage calldata a) { } catch { }
    }
}

==== Source: D1 ====
contract C {
    function f() public {
        try this.f() {}  catch (string memory memory x) { }
    }
}
==== Source: D2 ====
contract C {
    function f() public {
        try this.f() {}  catch (string memory calldata x) { }
    }
}
==== Source: D3 ====
contract C {
    function f() public {
        try this.f() {}  catch (string memory storage x) { }
    }
}

==== Source: E1 ====
contract C {
    function f() public {
        try this.f() {}  catch (string calldata memory x) { }
    }
}
==== Source: E2 ====
contract C {
    function f() public {
        try this.f() {}  catch (string calldata calldata x) { }
    }
}
==== Source: E3 ====
contract C {
    function f() public {
        try this.f() {}  catch (string calldata storage x) { }
    }
}

==== Source: G1 ====
contract C {
    function f() public {
        try this.f() {}  catch (string storage memory x) { }
    }
}
==== Source: G2 ====
contract C {
    function f() public {
        try this.f() {}  catch (string storage calldata x) { }
    }
}
==== Source: G3 ====
contract C {
    function f() public {
        try this.f() {}  catch (string storage storage x) { }
    }
}
// ----
// ParserError 3548: (A1:107-113): Location already specified.
// ParserError 3548: (A2:107-114): Location already specified.
// ParserError 3548: (A3:107-115): Location already specified.
// ParserError 3548: (B1:109-115): Location already specified.
// ParserError 3548: (B2:109-116): Location already specified.
// ParserError 3548: (B3:109-117): Location already specified.
// ParserError 3548: (C1:108-114): Location already specified.
// ParserError 3548: (C2:108-115): Location already specified.
// ParserError 3548: (C3:108-116): Location already specified.
// ParserError 3548: (D1:85-91): Location already specified.
// ParserError 3548: (D2:85-93): Location already specified.
// ParserError 3548: (D3:85-92): Location already specified.
// ParserError 3548: (E1:87-93): Location already specified.
// ParserError 3548: (E2:87-95): Location already specified.
// ParserError 3548: (E3:87-94): Location already specified.
// ParserError 3548: (G1:86-92): Location already specified.
// ParserError 3548: (G2:86-94): Location already specified.
// ParserError 3548: (G3:86-93): Location already specified.
