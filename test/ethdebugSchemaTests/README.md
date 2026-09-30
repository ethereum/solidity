# ETHDebug schema tests

These tests validate Solidity's ETHDebug output against a pinned checkout of the upstream `ethdebug/format` JSON Schemas.

The schema checkout is a git submodule at `test/ethdebugSchemaTests/ethdebug-format`. Initialize it with:

```bash
git submodule update --init test/ethdebugSchemaTests/ethdebug-format
```

Run the tests with:

```bash
test/ethdebugSchemaTests.py --solc-binary-path build/solc/solc
```

The runner discovers every test module in this directory and passes the compiler path to the test cases.
To run only some tests, use the `-k` option to specify a pattern, as with [`python -m unittest -k`](https://docs.python.org/3/library/unittest.html#cmdoption-unittest-k):

```bash
test/ethdebugSchemaTests.py --solc-binary-path build/solc/solc -k test_deployed_program_schema
```

The JSON inputs use `contentFile` entries to keep Solidity examples in regular `.sol` fixture files under `sources/`.
The tests expand those entries to Standard JSON `content` before invoking `solc`.

The suite only checks properties that hold for the output of any input.
The expected output for specific inputs is pinned down by the isoltest cases under `test/libsolidity/ethdebugTests/`; the resources of the cases in its `resources/` subdirectory are validated against the schemas here as well.

To update the schema version, bump the submodule commit and rerun this suite.
