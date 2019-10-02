contract Test {
    struct RecursiveStruct {
        RecursiveStruct[] vals;
    }

    function func() private pure {
        RecursiveStruct[1] memory val;
        val;
    }
}
