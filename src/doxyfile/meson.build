# Documentación usando Doxygen

items = [
    'README.md',
	'src',
    'scripts',
]

inputs = ''
foreach item : items
    inputs += '"' + join_paths(meson.source_root(), item) + '" '
endforeach

doxygen = find_program('doxygen')
practica = meson.source_root().split('/')[-1]
html = join_paths(get_option('prefix'), 'html')

config_DG = configuration_data()
config_DG.set('project_name', meson.project_name())
config_DG.set('project_version', meson.project_version())
config_DG.set('inputs', inputs)
config_DG.set('install_dir', html)
config_DG.set('dir_html', practica)
config_DG.set('style_sheet', join_paths(meson.source_root(), 'src/doxyfile', 'style.css'))

doxyfile = configure_file(
    input: 'Doxyfile.in',
    output: 'Doxyfile',
    configuration: config_DG,
)

doc = run_target('doc', command: [doxygen, doxyfile])
