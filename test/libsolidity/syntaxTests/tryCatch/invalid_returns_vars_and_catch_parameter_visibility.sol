==== Source: A ====
contract C {
    function f() public returns (uint) {
        try this.f() returns (uint public a) {
        } catch { }
    }
}
==== Source: B ====
contract C {
    function f() public returns (uint) {
        try this.f() returns (uint private a) {
        } catch { }
    }
}
==== Source: C ====
contract C {
    function f() public returns (uint) {
        try this.f() returns (uint external a) {
        } catch { }
    }
}
==== Source: D ====
contract C {
    function f() public returns (uint) {
        try this.f() returns (uint internal a) {
        } catch { }
    }
}
==== Source: E ====
contract C {
    function f() public returns (uint) {
        try this.f() returns (uint) {
        } catch Error(string memory public x) {
        }
    }
}
==== Source: F ====
contract C {
    function f() public returns (uint) {
        try this.f() returns (uint) {
        } catch Error(string memory private x) {
        }
    }
}
==== Source: G ====
contract C {
    function f() public returns (uint) {
        try this.f() returns (uint) {
        } catch Error(string memory external x) {
        }
    }
}
==== Source: H ====
contract C {
    function f() public returns (uint) {
        try this.f() returns (uint) {
        } catch Error(string memory internal x) {
        }
    }
}
==== Source: I ====
contract C {
    function f() public returns (uint) {
        try this.f() returns (uint) {
        } catch (bytes memory public x) {
        }
    }
}
==== Source: J ====
contract C {
    function f() public returns (uint) {
        try this.f() returns (uint) {
        } catch (bytes memory private x) {
        }
    }
}
==== Source: K ====
contract C {
    function f() public returns (uint) {
        try this.f() returns (uint) {
        } catch (bytes memory external x) {
        }
    }
}
==== Source: L ====
contract C {
    function f() public returns (uint) {
        try this.f() returns (uint) {
        } catch (bytes memory internal x) {
        }
    }
}
// ----
// ParserError 2314: (A:89-95): Expected ',' but got 'public'
// ParserError 2314: (B:89-96): Expected ',' but got 'private'
// ParserError 2314: (C:89-97): Expected ',' but got 'external'
// ParserError 2314: (D:89-97): Expected ',' but got 'internal'
// ParserError 2314: (E:128-134): Expected ',' but got 'public'
// ParserError 2314: (F:128-135): Expected ',' but got 'private'
// ParserError 2314: (G:128-136): Expected ',' but got 'external'
// ParserError 2314: (H:128-136): Expected ',' but got 'internal'
// ParserError 2314: (I:122-128): Expected ',' but got 'public'
// ParserError 2314: (J:122-129): Expected ',' but got 'private'
// ParserError 2314: (K:122-130): Expected ',' but got 'external'
// ParserError 2314: (L:122-130): Expected ',' but got 'internal'