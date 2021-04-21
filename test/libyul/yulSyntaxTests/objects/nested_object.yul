object "outer" {
        code { let x := mload(0) }
        data "x" "stringdata"
        object "inner" {
                code { mstore(0, 1) }
                object "inner inner" { code {} }
                data "innerx" "abc"
                data "innery" "def"
        }
}
// ----
