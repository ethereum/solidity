#!/usr/bin/env python2
#
# This script is used to generate the list of bugs per compiler version
# from the list of bugs.
# It updates the list in place and signals failure if there were changes.
# This makes it possible to use this script as part of CI to check
# that the list is up to date.

import os
import json
import re
import sys

def comp(version_string):
    return [int(c) for c in version_string.split('.')]

path = os.path.dirname(os.path.realpath(__file__))
with open(path + '/../docs/bugs.json') as bugsFile:
    bugs = json.load(bugsFile)

versions = {}
with open(path + '/../Changelog.md') as changelog:
    for line in changelog:
        m = re.search(r'^### (\S+) \((\d+-\d+-\d+)\)$', line)
        if m:
            versions[m.group(1)] = {}
            versions[m.group(1)]['released'] = m.group(2)

for v in versions:
    versions[v]['bugs'] = []
    for bug in bugs:
        if 'introduced' in bug and comp(bug['introduced']) > comp(v):
            continue
        if comp(bug['fixed']) <= comp(v):
            continue
        versions[v]['bugs'] += [bug['name']]

new_contents = json.dumps(versions, sort_keys=True, indent=4)
with open(path + '/../docs/bugs_by_version.json', 'r') as bugs_by_version:
    old_contents = bugs_by_version.read()
with open(path + '/../docs/bugs_by_version.json', 'w') as bugs_by_version:
    bugs_by_version.write(new_contents)
sys.exit(old_contents != new_contents)
