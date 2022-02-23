import os.path


def render_html_extra_templates(app):
    if app.builder.format != 'html':
        # Non-HTML builders do not provide .templates.render_string(). Note that a HTML
        # builder is still used also when building some other formats like json or epub.
        return

    for input_path, template_config in app.config.html_extra_templates.items():
        # Requiring absolute paths simplifies the implementation.
        if not os.path.isabs(input_path):
            raise RuntimeError(f"Template input path is not absolute: {input_path}")
        if not os.path.isabs(template_config['target']):
            raise RuntimeError(f"Template target path is not absolute: {template_config['target']}")

        with open(input_path, 'r', encoding='utf8') as input_file:
            # This runs Jinja2, which supports rendering {{ }} tags among other things.
            rendered_template = app.builder.templates.render_string(
                input_file.read(),
                template_config['context'],
            )

        with open(template_config['target'], 'w', encoding='utf8') as target_file:
            target_file.write(rendered_template)

        app.config.html_extra_path.append(template_config['target'])


def setup(app):
    app.add_config_value('html_extra_templates', default={}, rebuild='', types=dict)

    # Register a handler for the env-before-read-docs event. Any event that's fired before static
    # files get copied would do.
    app.connect(
        'env-before-read-docs',
        lambda app, env, docnames: render_html_extra_templates(app)
    )

    return {
        # NOTE: Need to access _raw_config here because setup() runs before app.config is ready.
        'version': app.config._raw_config['version'],  # pylint: disable=protected-access
        'parallel_read_safe': True,
        'parallel_write_safe': True,
    }
