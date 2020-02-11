contract c {
    uint[20] spacer;
    uint[] dynamic;

    function fill() public {
        for (uint i = 0; i < 21; ++i)
            dynamic.push(i + 1);
    }

    function halfClear() public {
        while (dynamic.length > 5)
            dynamic.pop();
    }

    function fullClear() public {
        delete dynamic;
    }
}

// ====
// compileViaYul: also
// ----
// fill() -> bytes(
// fill():"" -> ""
// halfClear() -> bytes(
// halfClear():"" -> ""
// fullClear() -> bytes(
// fullClear():"" -> ""
