object "object" {
    code {
        function main()
        {
            let m:i64, n:i32, p:i32, q:i64 := multireturn(1:i32, 2:i64, 3:i64, 4:i32)
        }

        function multireturn(a:i32, b:i64, c:i64, d:i32) -> x:i64, y:i32, z:i32, w:i64
        {
            x := b
            w := c

            y := a
            z := d
        }
    }
}
