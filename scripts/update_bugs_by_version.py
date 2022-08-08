#!/usr/bin/env python3
#
# This script is used to generate the list of bugs per compiler version
# from the list of bugs.
# It updates the list in place and signals failure if there were changes.
# This makes it possible to use this script as part of CI to check
# that the list is up to date.

import json
import re
from pathlib import Path

def comp(version_string):
    return [int(c) for c in version_string.split('.')]

root_path = Path(__file__).resolve().parent.parent

bugs = json.loads((root_path / 'docs/bugs.json').read_text(encoding='utf8'))

versions = {}
with (root_path / 'Changelog.md').open(encoding='utf8') as changelog:
    for line in changelog:
        m = re.search(r'^### (\S+) \((\d+-\d+-\d+)\)$', line)
        if m:
            versions[m.group(1)] = {}
            versions[m.group(1)]['released'] = m.group(2)

for key, value in versions.items():
    value['bugs'] = []
    for bug in bugs:
        if 'introduced' in bug and comp(bug['introduced']) > comp(key):
            continue
        if comp(bug['fixed']) <= comp(key):
            continue
        value['bugs'] += [bug['name']]

(root_path / 'docs/bugs_by_version.json').write_text(json.dumps(
    versions,
    sort_keys=True,
    indent=4,
    separators=(',', ': ')
), encoding='utf8')
