from copy import copy
import math
import re
import sys

batch_id = int(sys.argv[1])
batch_count = int(sys.argv[2])
assert batch_id >= 1

if batch_id > batch_count:
    sys.exit(0)

last_indent_level = 0
prefix = [""]
tests = []
suites = []
for line in sys.stdin:
    match = re.match(r'^( *)([^*]+)\*$', line)
    if match is None:
        print("No match")
    else:
        indent = match[1]
        name = match[2]

        assert len(indent) % 4 == 0
        indent_level = int(len(indent) / 4)

        if indent_level > last_indent_level:
            suites.append(copy(prefix))
            prefix.append(name)
        else:
            assert len(prefix) > 0
            if prefix[0] != "":
                tests.append(copy(prefix))

            for i in range(last_indent_level - indent_level + 1):
                prefix.pop()
            prefix.append(name)

        last_indent_level = indent_level

if len(prefix) > 0:
    assert prefix[0] != ""
    tests.append(copy(prefix))

start = math.ceil(len(tests) / batch_count) * (batch_id - 1)
end = math.ceil(len(tests) / batch_count) * batch_id

if start < len(tests):
    print(
        ':'.join([
            '/'.join(prefix)
            for i, prefix in enumerate(tests)
            if start <= i < min(end, len(tests))
        ])
    )
