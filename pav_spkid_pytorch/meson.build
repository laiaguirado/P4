# Scripts del proyecto

scripts = [
	'make_spk2idx.py',
	'test_nn.py',
	'train_nn.py',
	'utils.py',
]

foreach script: scripts
    source = join_paths(meson.source_root(), 'pav_spkid_pytorch', script)     # Ruta completa
    prog = script.split('.')[0]                                     # Nombre sin extensión
    dest = join_paths(get_option('prefix'), get_option('bindir'), prog)
    custom_target(script,
        input : source,
        output : prog,
        command : ['ln', '-sf', source, dest],
        build_by_default: true,
    )
endforeach
