#!/usr/bin/env python

from pexpect import spawn
from json import loads
from jinja2 import Environment, PackageLoader
import sys

class ContractDoc:
    def render(self, output):
        env = Environment(loader=PackageLoader('docsol', '.'))
        template = env.get_template('doc.template')
        output.write(template.render(contract=self._name, title=self._title, author=self._author, methods=self._methods))

    def __init__(self, contract_name, description_string):
        self._name = contract_name
        description = loads(description_string) 
        self._methods = description['methods']
        if 'author' in description:
            self._author = description['author']
        else:
            self._author = ''
        if 'title' in description:
            self._title = description['title']
        else:
            self._title = ''

class DocParser:
    # Tags
    name_tag   = '======='

    @staticmethod
    def parse(input_file): 
        readjson = False
        jsonbuff = []
        docname = ''
        docs = []
        for line in input_file:
            if not readjson:
                if line[0:7] == DocParser.name_tag:
                    docname = line.replace(DocParser.name_tag, "")[1:-1]
                    readjson = True
                    jsonbuff = []
            else:
                if line != '\n':
                    jsonbuff.append(line)
                else:
                    docs.append(ContractDoc(docname, ''.join(jsonbuff)))
                    readjson = False
        return docs

if __name__ == '__main__':
    docs = DocParser.parse(sys.stdin)
    for d in docs:
        d.render(sys.stdout)
