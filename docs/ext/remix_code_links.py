import base64
import docutils  # pragma pylint: disable=import-error

from sphinx.util import logging  # pragma pylint: disable=import-error

# NOTE: 2000 should generally be safe for all browsers, while 8000 for most of them.
MAX_SAFE_URL_LENGTH = 10000

logger = logging.getLogger(__name__)


def insert_node_before(child, new_sibling):
    assert child in child.parent.children

    for position, node in enumerate(child.parent.children):
        if node == child:
            child.parent.insert(position, new_sibling)
            break


def remix_code_url(source_code, language, solidity_version):
    # NOTE: base64 encoded data may contain +, = and / characters. Remix seems to handle them just
    # fine without any escaping.
    base64_encoded_source = base64.b64encode(source_code.encode('utf-8')).decode('ascii')
    return f"https://remix.ethereum.org/?#language={language}&version={solidity_version}&code={base64_encoded_source}"


def build_remix_link_node(url):
    reference_node = docutils.nodes.reference('', 'open in Remix', internal=False, refuri=url, target='_blank')
    reference_node['classes'].append('remix-link')

    paragraph_node = docutils.nodes.paragraph()
    paragraph_node['classes'].append('remix-link-container')
    paragraph_node.append(reference_node)
    return paragraph_node


def insert_remix_link(app, doctree, solidity_version):
    if app.builder.format != 'html' or app.builder.name == 'epub':
        return

    for literal_block_node in doctree.traverse(docutils.nodes.literal_block):
        assert 'language' in literal_block_node.attributes
        language = literal_block_node.attributes['language'].lower()
        if language not in ['solidity', 'yul']:
            continue

        text_nodes = list(literal_block_node.traverse(docutils.nodes.Text))
        assert len(text_nodes) == 1

        remix_url = remix_code_url(text_nodes[0], language, solidity_version)
        url_length = len(remix_url.encode('utf-8'))
        if url_length > MAX_SAFE_URL_LENGTH:
            logger.warning(
                "Remix URL generated from the code snippet exceeds the maximum safe URL length "
                " (%d > %d bytes).",
                url_length,
                MAX_SAFE_URL_LENGTH,
                location=(literal_block_node.source, literal_block_node.line),
            )

        insert_node_before(literal_block_node, build_remix_link_node(remix_url))


def setup(app):
    # NOTE: Need to access _raw_config here because setup() runs before app.config is ready.
    solidity_version = app.config._raw_config['version']  # pylint: disable=protected-access

    app.connect(
        'doctree-resolved',
        lambda app, doctree, docname: insert_remix_link(app, doctree, solidity_version)
    )

    return {
        'version': solidity_version,
        'parallel_read_safe': True,
        'parallel_write_safe': True,
    }
