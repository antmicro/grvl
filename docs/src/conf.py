from datetime import datetime
import sys
import os

from antmicro_sphinx_utils.defaults import (
    numfig_format,
    extensions as default_extensions,
    myst_enable_extensions as default_myst_enable_extensions,
    myst_fence_as_directive as default_myst_fence_as_directive,
    antmicro_html,
    antmicro_latex
)

project = u'Antmicro grvl'
basic_filename = u'grvl--user-guide'
authors = u'Antmicro'
copyright = f'{authors}, {datetime.now().year}'
version = ''
release = ''
sphinx_immaterial_override_builtin_admonitions = False
numfig = True
extensions = default_extensions + ['sphinx.ext.autodoc', 'breathe']
breathe_default_project = "grvl"
myst_enable_extensions = default_myst_enable_extensions
myst_fence_as_directive = default_myst_fence_as_directive
myst_substitutions = {
    "project": project
}
today_fmt = '%Y-%m-%d'
todo_include_todos=False
html_theme = 'sphinx_immaterial'
html_last_updated_fmt = today_fmt
html_show_sphinx = False
(
    html_logo,
    html_theme_options,
    html_context
) = antmicro_html(pdf_url=f"{basic_filename}.pdf")
html_title = project
(
    latex_elements,
    latex_documents,
    latex_logo,
    latex_additional_files
) = antmicro_latex(basic_filename, authors, project)
